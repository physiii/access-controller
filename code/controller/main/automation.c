#include "automation.h"
#include <time.h>

// Definition of global variables
#define MESSAGE_TIMEOUT 1
char wss_data_in[1800];
char wss_data_out[1800];
bool wss_data_out_ready = false;

bool run_relay = true;
bool get_time = false;

char token[700] = {0};
char device_id[100] = {0};
char wifi_ssid[32];
char wifi_password[64];
char server_ip[32];
char server_port[8];

bool disconnect_from_relay = false;
bool connect_to_relay = false;

const char *TAG = "open-automation";

struct ServiceMessage serviceMessage;
struct ServiceMessage clientMessage;


void init_automation_queues() {
    serviceMessage.queueCount = 0;
    serviceMessage.mutex = xSemaphoreCreateMutex();
    clientMessage.queueCount = 0;
    clientMessage.mutex = xSemaphoreCreateMutex();
}

cJSON *checkServiceMessage(char *eventType) {
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
            continue; // Skip this iteration if the necessary items are not present
        }

        if (strcmp(propertyItem->valuestring, action) == 0 && strcmp(serviceIdItem->valuestring, id) == 0) {
            return cJSON_Duplicate(currentMessage, 1);
        }
    }
    return NULL;
}

cJSON *checkServiceMessageByKey(char *key) {
    cJSON *null_payload = NULL;
    
    for (int i = 0; i < serviceMessage.queueCount; i++) {
        cJSON *currentMessage = serviceMessage.messageQueue[i];
        cJSON *item = cJSON_GetObjectItem(currentMessage, key);

        if (item) {
            return cJSON_Duplicate(currentMessage, 1);  // Return a copy of the found message
        }
    }

    return null_payload;  // Return NULL if no matching key found
}

void addServiceMessageToQueue(cJSON *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message to the queue.");
        return;
    }

    if (xSemaphoreTake(serviceMessage.mutex, portMAX_DELAY)) {
        size_t free_heap = esp_get_free_heap_size();
        if (free_heap < 10000) { // Increased threshold to 10KB
            ESP_LOGW(TAG, "Critical memory low (%zu bytes), clearing message queue", free_heap);
            for (int i = 0; i < serviceMessage.queueCount; i++) {
                if (serviceMessage.messageQueue[i]) {
                    cJSON_Delete(serviceMessage.messageQueue[i]);
                    serviceMessage.messageQueue[i] = NULL;
                }
            }
            serviceMessage.queueCount = 0;
        }

        if (serviceMessage.queueCount >= MAX_QUEUE_SIZE) {
            ESP_LOGW(TAG, "Service message queue full. Dropping the oldest message.");
            if (serviceMessage.messageQueue[0]) {
                cJSON_Delete(serviceMessage.messageQueue[0]);
            }
            for (int i = 1; i < serviceMessage.queueCount; i++) {
                serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
            }
             serviceMessage.messageQueue[serviceMessage.queueCount -1] = NULL;
            serviceMessage.queueCount--;
        }

        cJSON *duplicateMessage = cJSON_Duplicate(message, 1);
        if (!duplicateMessage) {
            ESP_LOGE(TAG, "Failed to duplicate message. Potential memory issue.");
        } else {
            serviceMessage.messageQueue[serviceMessage.queueCount] = duplicateMessage;
            serviceMessage.queueCount++;
             char *json_string = cJSON_PrintUnformatted(duplicateMessage);
            if (json_string) {
                ESP_LOGI(TAG, "Added message to service queue: %s", json_string);
                free(json_string);
            }
        }
        xSemaphoreGive(serviceMessage.mutex);
    }
}

void addClientMessageToQueue(cJSON *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message to client queue.");
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
             clientMessage.messageQueue[clientMessage.queueCount -1] = NULL;
            clientMessage.queueCount--;
        }

        cJSON *duplicateMessage = cJSON_Duplicate(message, 1);
        if (!duplicateMessage) {
            ESP_LOGE(TAG, "Failed to duplicate client message. Potential memory issue.");
        } else {
            clientMessage.messageQueue[clientMessage.queueCount] = duplicateMessage;
            clientMessage.queueCount++;
            char *json_string = cJSON_PrintUnformatted(duplicateMessage);
            if (json_string) {
                ESP_LOGI(TAG, "Added message to client queue: %s", json_string);
                free(json_string);
            }
        }
        xSemaphoreGive(clientMessage.mutex);
    }
} 

void addServerMessageToQueue(const char *message) {
    cJSON *msg_json = cJSON_CreateObject();
    cJSON_AddStringToObject(msg_json, "message", message);
    addServiceMessageToQueue(msg_json);
    cJSON_Delete(msg_json);
} 