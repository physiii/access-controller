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

struct ServiceMessage serviceMessage = {0};
struct ClientMessage clientMessage = {0};
struct ServerMessage serverMessage = {0};

cJSON *checkServiceMessage(char *eventType) {
    for (int i = 0; i < serviceMessage.queueCount; i++) {
        if (serviceMessage.messageQueue[i]) {
            cJSON *eventTypeItem = cJSON_GetObjectItem(serviceMessage.messageQueue[i], "eventType");
            if (eventTypeItem && strcmp(eventTypeItem->valuestring, eventType) == 0) {
                cJSON *payloadItem = cJSON_GetObjectItem(serviceMessage.messageQueue[i], "payload");
                if (payloadItem) {
                    cJSON *response = cJSON_Duplicate(payloadItem, 1);
                    ESP_LOGI(TAG, "Message processed for eventType: %s", eventType);

                    // Delete the found message and shift remaining messages
                    cJSON_Delete(serviceMessage.messageQueue[i]);
                    for (int j = i; j < serviceMessage.queueCount - 1; j++) {
                        serviceMessage.messageQueue[j] = serviceMessage.messageQueue[j + 1];
                    }
                    serviceMessage.messageQueue[serviceMessage.queueCount - 1] = NULL;
                    serviceMessage.queueCount--;
                    return response;
                }
            }
        }
    }
    return NULL;
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

    // Emergency memory check - clear queue if memory is critically low
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 5000) {
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
        ESP_LOGW(TAG, "Service message queue full. Dropping the oldest message to make space.");
        if (serviceMessage.messageQueue[0]) {
            cJSON_Delete(serviceMessage.messageQueue[0]);
        }
        serviceMessage.messageQueue[0] = NULL;
        for (int i = 1; i < serviceMessage.queueCount; i++) {
            serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
        }
        serviceMessage.queueCount--;
    }

    cJSON *duplicateMessage = cJSON_Duplicate(message, 1);
    if (!duplicateMessage) {
        ESP_LOGE(TAG, "Failed to duplicate message. Potential memory issue.");
        return;
    }

    serviceMessage.messageQueue[serviceMessage.queueCount] = duplicateMessage;
    serviceMessage.queueCount++;

    char *json_string = cJSON_PrintUnformatted(duplicateMessage);
    if (json_string) {
        ESP_LOGI(TAG, "Added message to queue: %s", json_string);
        free(json_string); // Free the printed string to prevent memory leak
    }
}

void addServerMessageToQueue(const char *message) {
    if (serverMessage.queueCount >= MAX_QUEUE_SIZE) {
        ESP_LOGW(TAG, "ServerMessage queue full, dropping message.");
        return;
    }
    strncpy(serverMessage.messageQueue[serverMessage.queueCount], message, sizeof(serverMessage.messageQueue[serverMessage.queueCount]) - 1);
    serverMessage.queueCount++;
}

void addClientMessageToQueue(char *message) {
    if (!message) {
        ESP_LOGE(TAG, "Attempting to add a null message to client queue.");
        return;
    }

    if (clientMessage.queueCount >= MAX_QUEUE_SIZE) {
        ESP_LOGW(TAG, "Client message queue full. Dropping oldest message.");
        // Shift all messages down
        for (int i = 1; i < clientMessage.queueCount; i++) {
            strncpy(clientMessage.messageQueue[i - 1], clientMessage.messageQueue[i], sizeof(clientMessage.messageQueue[i - 1]) - 1);
            clientMessage.messageQueue[i - 1][sizeof(clientMessage.messageQueue[i - 1]) - 1] = '\0';
        }
        clientMessage.queueCount--;
    }

    // Ensure message fits in buffer
    size_t message_len = strlen(message);
    if (message_len >= sizeof(clientMessage.messageQueue[0])) {
        ESP_LOGW(TAG, "Message too long (%zu chars), truncating", message_len);
        message_len = sizeof(clientMessage.messageQueue[0]) - 1;
    }

    strncpy(clientMessage.messageQueue[clientMessage.queueCount], message, message_len);
    clientMessage.messageQueue[clientMessage.queueCount][message_len] = '\0';
    clientMessage.queueCount++;

    ESP_LOGI(TAG, "addClientMessageToQueue (%d) %s", clientMessage.queueCount - 1, message);
}

void serviceMessageTask(void *pvParameter) {
    while (1) {
        if (serviceMessage.queueCount > 0) {
            if (serviceMessage.messageQueue[0]) {
                // Process message if needed
            }
        }
        vTaskDelay(SERVICE_LOOP_SHORT);
    }
}

void serverMessageTask(void *pvParameter) {
    while (1) {
        if (!serverMessage.readyToSend) {
            if (serverMessage.queueCount > 0) {
                strncpy(serverMessage.message, serverMessage.messageQueue[0], sizeof(serverMessage.message) - 1);
                serverMessage.readyToSend = true;

                for (int i = 1; i < serverMessage.queueCount; ++i) {
                    strncpy(serverMessage.messageQueue[i - 1], serverMessage.messageQueue[i], sizeof(serverMessage.messageQueue[i - 1]) - 1);
                }
                serverMessage.queueCount--;
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void clientMessageTask(void *pvParameter) {
    while (1) {
        if (!clientMessage.readyToSend) {
            if (clientMessage.queueCount > 0) {
                strncpy(clientMessage.message, clientMessage.messageQueue[0], sizeof(clientMessage.message) - 1);
                clientMessage.readyToSend = true;
                
                // Shift all messages down
                for (int i = 1; i < clientMessage.queueCount; i++) {
                    strncpy(clientMessage.messageQueue[i - 1], clientMessage.messageQueue[i], sizeof(clientMessage.messageQueue[i - 1]) - 1);
                }
                clientMessage.queueCount--;
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
} 