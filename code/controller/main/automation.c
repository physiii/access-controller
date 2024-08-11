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

const char *OA_TAG = "open-automation";

struct ServiceMessage serviceMessage = {0};
struct ClientMessage clientMessage = {0};
struct ServerMessage serverMessage = {0};

cJSON *checkServiceMessage(char *eventType) {
    if (serviceMessage.queueCount > 0 && serviceMessage.messageQueue[0]) {
        cJSON *eventTypeItem = cJSON_GetObjectItem(serviceMessage.messageQueue[0], "eventType");
        if (eventTypeItem && strcmp(eventTypeItem->valuestring, eventType) == 0) {
            cJSON *payloadItem = cJSON_GetObjectItem(serviceMessage.messageQueue[0], "payload");
            if (payloadItem) {
                cJSON *response = cJSON_Duplicate(payloadItem, 1);
                ESP_LOGI(OA_TAG, "Message processed for eventType: %s", eventType);

                cJSON_Delete(serviceMessage.messageQueue[0]);
                serviceMessage.messageQueue[0] = NULL;
                for (int i = 1; i < serviceMessage.queueCount; i++) {
                    serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
                    serviceMessage.messageTimestamps[i - 1] = serviceMessage.messageTimestamps[i];
                }
                serviceMessage.queueCount--;
                return response;
            }
        }
    }
    return NULL;
}

cJSON *checkServiceMessageByAction(char *id, char *action) {
    if (!id || !action) {
        ESP_LOGE(OA_TAG, "Invalid or null ID/action provided.");
        return NULL;
    }

    for (int i = 0; i < serviceMessage.queueCount; i++) {
        cJSON *currentMessage = serviceMessage.messageQueue[i];
        if (!currentMessage) {
            ESP_LOGE(OA_TAG, "Null message encountered at index %d.", i);
            continue;
        }

        cJSON *propertyItem = cJSON_GetObjectItem(currentMessage, "property");
        cJSON *serviceIdItem = cJSON_GetObjectItem(currentMessage, "service_id");

        if (!propertyItem || !serviceIdItem) {
            // ESP_LOGW(OA_TAG, "Missing 'property' or 'service_id' in message at index %d.", i);
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
        ESP_LOGE(OA_TAG, "Attempting to add a null message to the queue.");
        return;
    }

    if (serviceMessage.queueCount >= MAX_QUEUE_SIZE) {
        ESP_LOGW(OA_TAG, "Service message queue full. Dropping the oldest message to make space.");
        cJSON_Delete(serviceMessage.messageQueue[0]);
        serviceMessage.messageQueue[0] = NULL;
        for (int i = 1; i < serviceMessage.queueCount; i++) {
            serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
            serviceMessage.messageTimestamps[i - 1] = serviceMessage.messageTimestamps[i];
        }
        serviceMessage.queueCount--;
    }

    cJSON *duplicateMessage = cJSON_Duplicate(message, 1);
    if (!duplicateMessage) {
        ESP_LOGE(OA_TAG, "Failed to duplicate the message for the queue.");
        return;
    }

    serviceMessage.messageQueue[serviceMessage.queueCount] = duplicateMessage;
    serviceMessage.messageTimestamps[serviceMessage.queueCount] = time(NULL);
    serviceMessage.queueCount++;

    char *msg = cJSON_PrintUnformatted(duplicateMessage);
    ESP_LOGI(OA_TAG, "Added message to queue: %s", msg);
    free(msg);
}

void addServerMessageToQueue(const char *message) {
    if (serverMessage.queueCount >= MAX_QUEUE_SIZE) {
        ESP_LOGW(OA_TAG, "ServerMessage queue full, dropping message.");
        return;
    }
    strncpy(serverMessage.messageQueue[serverMessage.queueCount], message, sizeof(serverMessage.messageQueue[serverMessage.queueCount]) - 1);
    serverMessage.queueCount++;
}

void addClientMessageToQueue(const char *message) {
    if (clientMessage.queueCount >= MAX_QUEUE_SIZE) {
        ESP_LOGW(OA_TAG, "ClientMessage queue full, dropping message.");
        return;
    }
    strncpy(clientMessage.messageQueue[clientMessage.queueCount], message, sizeof(clientMessage.messageQueue[clientMessage.queueCount]) - 1);
    ESP_LOGI(OA_TAG, "addClientMessageToQueue (%d) %s\n", clientMessage.queueCount, message);
    clientMessage.queueCount++;

    addServerMessageToQueue(message);
}

void serviceMessageTask(void *pvParameter) {
    while (1) {
        if (serviceMessage.queueCount > 0) {
            time_t currentTime = time(NULL);
            time_t messageAge = currentTime - serviceMessage.messageTimestamps[0];

            if (messageAge > MESSAGE_TIMEOUT) {
                cJSON_Delete(serviceMessage.messageQueue[0]);
                serviceMessage.messageQueue[0] = NULL;
                for (int i = 1; i < serviceMessage.queueCount; i++) {
                    serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
                    serviceMessage.messageTimestamps[i - 1] = serviceMessage.messageTimestamps[i];
                }
                serviceMessage.queueCount--;
            } else if (serviceMessage.messageQueue[0]) {
                // ESP_LOGI(OA_TAG, "Processing message: %s", cJSON_PrintUnformatted(serviceMessage.messageQueue[0]));
                // cJSON *message = serviceMessage.messageQueue[0];
                // cJSON_Delete(message);
                // serviceMessage.messageQueue[0] = NULL;
                // for (int i = 1; i < serviceMessage.queueCount; i++) {
                //     serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
                //     serviceMessage.messageTimestamps[i - 1] = serviceMessage.messageTimestamps[i];
                // }
                // serviceMessage.queueCount--;
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
                clientMessage.queueCount--;
                strncpy(clientMessage.message, clientMessage.messageQueue[clientMessage.queueCount], sizeof(clientMessage.message) - 1);
                clientMessage.readyToSend = true;
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void automationInit() {
    // Initialize the service message queue
    serviceMessage.queueCount = 0;
    serviceMessage.read = false;

    // Initialize the client message queue
    clientMessage.queueCount = 0;
    clientMessage.readyToSend = false;

    // Initialize the server message queue
    serverMessage.queueCount = 0;
    serverMessage.readyToSend = false;

    TaskHandle_t serviceMessageTaskHandle = NULL;
    TaskHandle_t clientMessageTaskHandle = NULL;
    TaskHandle_t serverMessageTaskHandle = NULL;

    xTaskCreate(serviceMessageTask, "serviceMessageTask", 5000, NULL, 10, &serviceMessageTaskHandle);
    xTaskCreate(clientMessageTask, "clientMessageTask", 5000, NULL, 10, &clientMessageTaskHandle);
    xTaskCreate(serverMessageTask, "serverMessageTask", 5000, NULL, 10, &serverMessageTaskHandle);
}