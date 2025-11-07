#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "automation.h"
#include "services/log_store.h"

static const char *TAG = "open-automation";

// Global variables needed by other modules
char token[700] = {0};
char device_id[100] = {0};
char server_ip[32];
char server_port[8];

struct ServiceMessage serviceMessage;
struct ServiceMessage clientMessage;

#ifdef __cplusplus
extern "C" {
#endif
bool ws_has_active_clients(void);
#ifdef __cplusplus
}
#endif

#define SYSTEM_LOG_CAPACITY LOG_STORE_CAPACITY
#define SYSTEM_LOG_MESSAGE_MAX LOG_STORE_MESSAGE_MAX
#define SYSTEM_LOG_EXPORT_MAX 20

typedef struct {
    uint64_t timestamp_ms;
    int64_t unix_time;
    char message[SYSTEM_LOG_MESSAGE_MAX];
} system_log_entry_t;

static system_log_entry_t s_system_logs[SYSTEM_LOG_CAPACITY];
static size_t s_system_log_count = 0;
static size_t s_system_log_next = 0;
static bool s_ntp_synced = false;
static int64_t s_unix_offset = 0;
static bool s_shutdown_handler_registered = false;

static void automation_shutdown_handler(void) {
    log_store_flush_now();
}

static void record_log_internal(const char *message) {
    if (!message || message[0] == '\0') {
        return;
    }

    uint64_t now_ms = esp_timer_get_time() / 1000ULL;
    system_log_entry_t *entry = &s_system_logs[s_system_log_next];
    entry->timestamp_ms = now_ms;
    strlcpy(entry->message, message, SYSTEM_LOG_MESSAGE_MAX);
    entry->unix_time = s_ntp_synced ? s_unix_offset + (int64_t)(now_ms / 1000ULL) : 0;

    s_system_log_next = (s_system_log_next + 1) % SYSTEM_LOG_CAPACITY;
    if (s_system_log_count < SYSTEM_LOG_CAPACITY) {
        s_system_log_count++;
    }

    if (log_store_append(entry->timestamp_ms, entry->unix_time, message) != 0) {
        ESP_LOGW(TAG, "Failed to persist log message: %s", message);
    }
}

void automation_record_log(const char *message) {
    static char last_message[SYSTEM_LOG_MESSAGE_MAX];
    static uint64_t last_timestamp_ms = 0;

    if (!message) {
        return;
    }

    uint64_t now_ms = esp_timer_get_time() / 1000ULL;
    if (last_message[0] != '\0') {
        if (strncmp(last_message, message, SYSTEM_LOG_MESSAGE_MAX) == 0) {
            if (now_ms - last_timestamp_ms < 5000ULL) {
                return;
            }
        }
    }

    record_log_internal(message);
    strlcpy(last_message, message, SYSTEM_LOG_MESSAGE_MAX);
    last_timestamp_ms = now_ms;
}

void automation_update_unix_time(int64_t unix_time_seconds) {
    if (unix_time_seconds <= 0) {
        return;
    }

    uint64_t now_ms = esp_timer_get_time() / 1000ULL;
    int64_t new_offset = unix_time_seconds - (int64_t)(now_ms / 1000ULL);
    bool was_synced = s_ntp_synced;
    int64_t delta_seconds = 0;

    if (was_synced) {
        delta_seconds = new_offset - s_unix_offset;
    }

    s_ntp_synced = true;
    s_unix_offset = new_offset;

    if (!was_synced) {
        automation_record_log("Time synchronised via network");
    } else {
        char message[SYSTEM_LOG_MESSAGE_MAX];
        snprintf(message, sizeof(message), "Time re-synced (delta=%+llds unix=%lld)",
                 (long long)delta_seconds, (long long)unix_time_seconds);
        automation_record_log(message);
    }
}

static const char *reset_reason_to_string(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_POWERON: return "Power-on";
        case ESP_RST_EXT: return "External";
        case ESP_RST_SW: return "Software";
        case ESP_RST_PANIC: return "Panic";
        case ESP_RST_INT_WDT: return "Interrupt WDT";
        case ESP_RST_TASK_WDT: return "Task WDT";
        case ESP_RST_WDT: return "Other WDT";
        case ESP_RST_DEEPSLEEP: return "Deep sleep";
        case ESP_RST_BROWNOUT: return "Brownout";
        case ESP_RST_SDIO: return "SDIO";
        default: return "Unknown";
    }
}

void automation_log_boot_event(void) {
    char message[128];
    esp_reset_reason_t reason = esp_reset_reason();
    snprintf(message, sizeof(message), "Boot complete (reset reason: %s)", reset_reason_to_string(reason));
    automation_record_log(message);

    switch (reason) {
        case ESP_RST_BROWNOUT:
            automation_record_log("Warning: Brownout reset detected");
            break;
        case ESP_RST_PANIC:
            automation_record_log("Warning: CPU panic reset detected");
            break;
        case ESP_RST_TASK_WDT:
        case ESP_RST_INT_WDT:
        case ESP_RST_WDT:
            automation_record_log("Warning: Watchdog reset detected");
            break;
        default:
            break;
    }
}

cJSON *system_logs_snapshot(void) {
    cJSON *array = cJSON_CreateArray();
    if (!array) {
        return NULL;
    }

    stored_log_entry_t persisted[SYSTEM_LOG_EXPORT_MAX];
    size_t persisted_count = log_store_read(persisted, SYSTEM_LOG_EXPORT_MAX);

    for (size_t i = 0; i < persisted_count; i++) {
        cJSON *entry = cJSON_CreateObject();
        if (!entry) {
            continue;
        }
        cJSON_AddNumberToObject(entry, "timestamp", (double)persisted[i].timestamp_ms);
        if (persisted[i].unix_time > 0) {
            cJSON_AddNumberToObject(entry, "unixTime", (double)persisted[i].unix_time);
        }
        cJSON_AddStringToObject(entry, "message", persisted[i].message ? persisted[i].message : "");
        cJSON_AddItemToArray(array, entry);
    }

    log_store_free_entries(persisted, persisted_count);

    return array;
}

void init_automation_queues() {
    log_store_init();
    if (!s_shutdown_handler_registered) {
        if (esp_register_shutdown_handler(automation_shutdown_handler) == ESP_OK) {
            s_shutdown_handler_registered = true;
        } else {
            ESP_LOGW(TAG, "Failed to register shutdown handler for log flushing");
        }
    }
    serviceMessage.queueCount = 0;
    serviceMessage.mutex = xSemaphoreCreateMutex();
    clientMessage.queueCount = 0;
    clientMessage.mutex = xSemaphoreCreateMutex();
}

void checkServiceMessage(void) {
    if (serviceMessage.queueCount > 0) {
        if (xSemaphoreTake(serviceMessage.mutex, portMAX_DELAY)) {
            if (serviceMessage.queueCount > 0) {
                cJSON *message = serviceMessage.messageQueue[0];
                
                if (message) {
                    cJSON *eventType = cJSON_GetObjectItem(message, "eventType");
                    if (eventType && cJSON_IsString(eventType)) {
                        ESP_LOGI(TAG, "Message processed for eventType: %s", eventType->valuestring);
                    }
                    
                    cJSON_Delete(message);
                    
                    // Shift queue
                    for (int i = 1; i < serviceMessage.queueCount; i++) {
                        serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
                    }
                    serviceMessage.messageQueue[serviceMessage.queueCount - 1] = NULL;
                    serviceMessage.queueCount--;
                }
            }
            xSemaphoreGive(serviceMessage.mutex);
        }
    }
}

cJSON *checkServiceMessageByType(char *eventType) {
    cJSON *response = NULL;
    if (xSemaphoreTake(serviceMessage.mutex, portMAX_DELAY)) {
        for (int i = 0; i < serviceMessage.queueCount; i++) {
            if (serviceMessage.messageQueue[i]) {
                cJSON *eventTypeItem = cJSON_GetObjectItem(serviceMessage.messageQueue[i], "eventType");
                if (eventTypeItem && strcmp(eventTypeItem->valuestring, eventType) == 0) {
                    cJSON *payloadItem = cJSON_GetObjectItem(serviceMessage.messageQueue[i], "payload");
                    if (payloadItem) {
                        response = cJSON_Duplicate(payloadItem, 1);
                        ESP_LOGI(TAG, "Message processed for eventType: %s", eventType);
                        
                        cJSON_Delete(serviceMessage.messageQueue[i]);
                        for (int j = i; j < serviceMessage.queueCount - 1; j++) {
                            serviceMessage.messageQueue[j] = serviceMessage.messageQueue[j + 1];
                        }
                        serviceMessage.messageQueue[serviceMessage.queueCount - 1] = NULL;
                        serviceMessage.queueCount--;
                        goto cleanup;
                    }
                }
            }
        }
    cleanup:
        xSemaphoreGive(serviceMessage.mutex);
    }
    return response;
}

cJSON *checkServiceMessageByAction(char *id, char *action) {
    if (!id || !action) {
        ESP_LOGE(TAG, "Invalid or null ID/action provided.");
        return NULL;
    }

    for (int i = 0; i < serviceMessage.queueCount; i++) {
        cJSON *currentMessage = serviceMessage.messageQueue[i];
        if (!currentMessage) {
            ESP_LOGE(TAG, "Null message encountered at index %d.", i);
            continue;
        }

        cJSON *propertyItem = cJSON_GetObjectItem(currentMessage, "property");
        cJSON *serviceIdItem = cJSON_GetObjectItem(currentMessage, "service_id");

        if (!propertyItem || !serviceIdItem) {
            continue;
        }

        if (strcmp(propertyItem->valuestring, action) == 0 && strcmp(serviceIdItem->valuestring, id) == 0) {
            return cJSON_Duplicate(currentMessage, 1);
        }
    }
    return NULL;
}

cJSON *checkServiceMessageByKey(char *key) {
    for (int i = 0; i < serviceMessage.queueCount; i++) {
        cJSON *currentMessage = serviceMessage.messageQueue[i];
        cJSON *item = cJSON_GetObjectItem(currentMessage, key);

        if (item) {
            return cJSON_Duplicate(currentMessage, 1);
        }
    }
    return NULL;
}

void addServiceMessageToQueue(cJSON *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message to service queue.");
        return;
    }

    // Emergency memory check
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 10240) { // 10KB threshold
        ESP_LOGW(TAG, "Low memory (%d bytes), clearing service queue", free_heap);
        automation_record_log("Service queue cleared due to low memory");
        if (xSemaphoreTake(serviceMessage.mutex, portMAX_DELAY)) {
            for (int i = 0; i < serviceMessage.queueCount; i++) {
                if (serviceMessage.messageQueue[i]) {
                    cJSON_Delete(serviceMessage.messageQueue[i]);
                    serviceMessage.messageQueue[i] = NULL;
                }
            }
            serviceMessage.queueCount = 0;
            xSemaphoreGive(serviceMessage.mutex);
        }
        // We cannot safely keep the incoming message either
        cJSON_Delete(message);
        return;
    }

    if (xSemaphoreTake(serviceMessage.mutex, portMAX_DELAY)) {
        if (serviceMessage.queueCount >= MAX_QUEUE_SIZE) {
            ESP_LOGW(TAG, "Service message queue full. Dropping the oldest message.");
            if (serviceMessage.messageQueue[0]) {
                cJSON_Delete(serviceMessage.messageQueue[0]);
            }
            for (int i = 1; i < serviceMessage.queueCount; i++) {
                serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
            }
            serviceMessage.messageQueue[serviceMessage.queueCount - 1] = NULL;
            serviceMessage.queueCount--;
            automation_record_log("Service queue full: dropped oldest message");
        }

        // Take ownership without duplicating to reduce heap pressure
        serviceMessage.messageQueue[serviceMessage.queueCount] = message;
        serviceMessage.queueCount++;
        ESP_LOGI(TAG, "Added message to service queue.");
        xSemaphoreGive(serviceMessage.mutex);
    }
}

void addClientMessageToQueue(cJSON *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message to client queue.");
        return;
    }

    if (!ws_has_active_clients()) {
        return;
    }

    if (xSemaphoreTake(clientMessage.mutex, portMAX_DELAY)) {
        if (clientMessage.queueCount >= MAX_QUEUE_SIZE) {
            ESP_LOGW(TAG, "Client message queue full. Dropping oldest message.");
            if (clientMessage.messageQueue[0]) {
                cJSON_Delete(clientMessage.messageQueue[0]);
            }
            for (int i = 1; i < clientMessage.queueCount; i++) {
                clientMessage.messageQueue[i - 1] = clientMessage.messageQueue[i];
            }
            clientMessage.messageQueue[clientMessage.queueCount - 1] = NULL;
            clientMessage.queueCount--;
            automation_record_log("Client queue full: dropped oldest message");
        }

        cJSON *duplicateMessage = cJSON_Duplicate(message, 1);
        if (!duplicateMessage) {
            ESP_LOGE(TAG, "Failed to duplicate client message. Potential memory issue.");
        } else {
            clientMessage.messageQueue[clientMessage.queueCount] = duplicateMessage;
            clientMessage.queueCount++;
            ESP_LOGI(TAG, "Added message to client queue.");
        }
        xSemaphoreGive(clientMessage.mutex);
    }
}

void addServerMessageToQueue(const char *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message string to server queue.");
        return;
    }

    cJSON *json_msg = cJSON_Parse(message);
    if (!json_msg) {
        ESP_LOGE(TAG, "Failed to parse server message as JSON: %s", message);
        return;
    }

    // Transfer ownership to service queue; it will free the object
    addServiceMessageToQueue(json_msg);
} 