#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVICE_LOOP 100
#define SERVICE_LOOP_SHORT 10
#define STRIKE 0
#define MAX_QUEUE_SIZE 20 // Define a constant for the maximum queue size

#if STRIKE
#define USE_MCP23017 0
#else
#define USE_MCP23017 1
#endif

#define NO_DATA_TIMEOUT_SEC 5

char wss_data_in[1800];
char wss_data_out[1800];
bool wss_data_out_ready = false;

bool run_relay = true;
bool get_time = false;

char token[700];
char device_id[100];
bool disconnect_from_relay = false;
bool connect_to_relay = true;

static const char *TAG = "open-automation";

struct ServiceMessage
{
    cJSON *message;
    cJSON *messageQueue[MAX_QUEUE_SIZE];
    bool read;
    int timeout;
    int queueCount;
};

struct ClientMessage
{
    char message[1000];
    char messageQueue[MAX_QUEUE_SIZE][1000];
    bool readyToSend;
    int timeout;
    int queueCount;
};

struct ServerMessage
{
    char message[1000];
    char messageQueue[MAX_QUEUE_SIZE][1000];
    bool readyToSend;
    int timeout;
    int queueCount;
};

struct ServiceMessage serviceMessage = {0};
struct ClientMessage clientMessage = {0};
struct ServerMessage serverMessage = {0};

void set_mcp_io(uint8_t, bool);
bool get_mcp_io(uint8_t);

cJSON *checkServiceMessage(char *eventType)
{
    cJSON *null_payload = NULL;
    if (serviceMessage.read)
        return null_payload;

    if (serviceMessage.message == NULL)
        return null_payload;

    cJSON *eventTypeItem = cJSON_GetObjectItem(serviceMessage.message, "eventType");
    if (!eventTypeItem)
        return null_payload;

    char type[50] = "";
    strncpy(type, eventTypeItem->valuestring, sizeof(type) - 1);

    if (strcmp(type, eventType))
        return null_payload;

    cJSON *payloadItem = cJSON_GetObjectItem(serviceMessage.message, "payload");
    if (!payloadItem)
        return null_payload;

    cJSON *payload = payloadItem;

    serviceMessage.read = true;
    return payload;
}

cJSON *checkServiceMessageByAction(char *id, char *action)
{
    cJSON *payload = NULL;

    if (serviceMessage.message == NULL || serviceMessage.read)
        return payload;

    if (cJSON_GetObjectItem(serviceMessage.message,"property") == NULL)
        return payload;

    char property[250];
    sprintf(property, "%s", cJSON_GetObjectItem(serviceMessage.message,"property")->valuestring);
    if (strcmp(property, action)) return payload;

    // printf("current serviceMessage: %s\n", cJSON_PrintUnformatted(serviceMessage.message));

    cJSON *service_id = cJSON_GetObjectItem(serviceMessage.message, "service_id");
    if (!service_id) {
        return payload;
	}

    char _id[50] = "";
    strncpy(_id, service_id->valuestring, sizeof(_id) - 1);

    if (strcmp(id, _id)) {
        printf("id not match\n");
		return payload;
	}

    payload = cJSON_Duplicate(serviceMessage.message, 1);
    serviceMessage.read = true;
    // printf("returning duplicated payload: %s\n", cJSON_PrintUnformatted(payload));
	return payload;
}

cJSON *checkServiceMessageByKey(char *key)
{
    cJSON *null_payload = NULL;
    char _key[250] = "";

    if (serviceMessage.message == NULL || serviceMessage.read)
        return null_payload;

    cJSON *item = cJSON_GetObjectItem(serviceMessage.message, key);
    if (item && item->string) {
        sprintf(_key, "%s", item->string);
    }

    if (strcmp(key, _key)) {
        return null_payload;
    }

    serviceMessage.read = true;
    return cJSON_Duplicate(serviceMessage.message, 1);
}


void addServiceMessageToQueue(cJSON *message)
{
    if(message == NULL) {
        ESP_LOGE(TAG, "Received NULL message. Skipping...");
        return;
    }

    if (serviceMessage.queueCount >= MAX_QUEUE_SIZE) {
        // Handle full queue, e.g., by deleting the oldest message
        cJSON_Delete(serviceMessage.messageQueue[0]);
        for (int i = 1; i < MAX_QUEUE_SIZE; i++) {
            serviceMessage.messageQueue[i-1] = serviceMessage.messageQueue[i];
        }
        serviceMessage.queueCount--;
    }

    cJSON *duplicateMessage = cJSON_Duplicate(message, 1);

    if(duplicateMessage == NULL) {
        ESP_LOGE(TAG, "Failed to duplicate JSON message. Skipping...");
        return;
    }

    serviceMessage.messageQueue[serviceMessage.queueCount] = duplicateMessage;
    serviceMessage.queueCount++;

    char *msg = cJSON_PrintUnformatted(message);  // For logging
    ESP_LOGI(TAG, "addServiceMessageToQueue (%d) %s\n", serviceMessage.queueCount, msg);
    free(msg);  // Free the allocated string
}

void addServerMessageToQueue(const char *message)
{
    if (serverMessage.queueCount >= MAX_QUEUE_SIZE)
    {
        ESP_LOGW(TAG, "ServerMessage queue full, dropping message.");
        return;
    }
    strncpy(serverMessage.messageQueue[serverMessage.queueCount], message, sizeof(serverMessage.messageQueue[serverMessage.queueCount]) - 1);
    ESP_LOGI(TAG, "addServerMessageToQueue (%d) %s\n", serverMessage.queueCount, message);
    serverMessage.queueCount++;
}

void addClientMessageToQueue(const char *message)
{
    if (clientMessage.queueCount >= MAX_QUEUE_SIZE)
    {
        ESP_LOGW(TAG, "ClientMessage queue full, dropping message.");
        return;
    }
    strncpy(clientMessage.messageQueue[clientMessage.queueCount], message, sizeof(clientMessage.messageQueue[clientMessage.queueCount]) - 1);
    ESP_LOGI(TAG, "addClientMessageToQueue (%d) %s\n", clientMessage.queueCount, message);
    clientMessage.queueCount++;

    addServerMessageToQueue(message);
}

static void serviceMessageTask(void *pvParameter)
{
    int cnt = 0;
    while (1)
    {
        if (serviceMessage.read)
        {
            cnt = 0;
            if (serviceMessage.queueCount > 0)
            {
                // Free any existing message
                if (serviceMessage.message)
                {
                    cJSON_Delete(serviceMessage.message);
                    serviceMessage.message = NULL;
                }

                // Dequeue the next message from the queue
                serviceMessage.message = serviceMessage.messageQueue[0];
                serviceMessage.read = false;

                // Shift all remaining messages in the queue down
                for (int i = 1; i < serviceMessage.queueCount; i++)
                {
                    serviceMessage.messageQueue[i - 1] = serviceMessage.messageQueue[i];
                }
                // Null the last position in the queue
                serviceMessage.messageQueue[serviceMessage.queueCount - 1] = NULL;
                serviceMessage.queueCount--;

                // No need to delete the message here as it is now in serviceMessage.message and will be deleted next time
            }
        }
        else if (cnt > 10)
        {
            ESP_LOGE(TAG, "serviceMessage timeout reached.\n");
            cnt = 0;

            // Free any existing message
            if (serviceMessage.message)
            {
                cJSON_Delete(serviceMessage.message);
                serviceMessage.message = NULL;
            }

            serviceMessage.read = true;
        }
        else
        {
            cnt++;
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

static void serverMessageTask(void *pvParameter)
{
    while (1)
    {
        if (!serverMessage.readyToSend)
        {
            if (serverMessage.queueCount > 0)
            {
                // Dequeue the message from the queue and store it in serverMessage.message
                strncpy(serverMessage.message, serverMessage.messageQueue[0], sizeof(serverMessage.message) - 1);
                serverMessage.readyToSend = true;

                // Shift all the messages up by one position
                for (int i = 1; i < serverMessage.queueCount; ++i)
                {
                    strncpy(serverMessage.messageQueue[i - 1], serverMessage.messageQueue[i], sizeof(serverMessage.messageQueue[i - 1]) - 1);
                }

                serverMessage.queueCount--;

                // ESP_LOGI(TAG, "serverMessageTask (%d)\n", serverMessage.queueCount);
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

static void clientMessageTask(void *pvParameter)
{
    while (1)
    {
        if (!clientMessage.readyToSend)
        {
            if (clientMessage.queueCount > 0)
            {
                clientMessage.queueCount--;
                // Using strncpy instead of strcpy
                strncpy(clientMessage.message, clientMessage.messageQueue[clientMessage.queueCount], sizeof(clientMessage.message) - 1);
                clientMessage.readyToSend = true;
                ESP_LOGI(TAG, "clientMessageTask (%d)\n", clientMessage.queueCount);
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}
