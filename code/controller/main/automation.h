#define SERVICE_LOOP 			100
#define SERVICE_LOOP_SHORT		10
#define STRIKE					0
#if STRIKE
	#define USE_MCP23017		0
#else
	#define USE_MCP23017		1
#endif

struct ServiceMessage
{
	cJSON *message;
	cJSON *messageQueue[100];  // Pointers instead of objects
	bool read;
	int timeout;
	int queueCount;
};

struct ClientMessage
{
	char message[1000];
	char messageQueue[20][1000];
	bool readyToSend;
	int timeout;
	int queueCount;
};

struct ServerMessage
{
	char message[1000];
	char messageQueue[20][1000];
	bool readyToSend;
	int timeout;
	int queueCount;
};

struct ServiceMessage serviceMessage = {0};  // Initialize struct
struct ClientMessage clientMessage = {0};  // Initialize struct
struct ServerMessage serverMessage = {0};  // Initialize struct

void set_mcp_io(uint8_t, bool);
bool get_mcp_io(uint8_t);

cJSON * checkServiceMessage(char *eventType)
{
	cJSON *null_payload = NULL;
	if (serviceMessage.read) return null_payload;

	if (serviceMessage.message == NULL) return null_payload;
	if (!cJSON_GetObjectItem(serviceMessage.message,"eventType")) return null_payload;

	char type[50] = "";
	strcpy(type, cJSON_GetObjectItem(serviceMessage.message,"eventType")->valuestring);

	if (strcmp(type, eventType)) return null_payload;
	if (!cJSON_GetObjectItem(serviceMessage.message,"payload")) return null_payload;

	cJSON *payload = cJSON_GetObjectItem(serviceMessage.message,"payload");

	serviceMessage.read = true;
	return payload;
}

void addServiceMessageToQueue (cJSON *message)
{
	serviceMessage.queueCount++;
	serviceMessage.messageQueue[serviceMessage.queueCount] = cJSON_Duplicate(message, 1); // Duplicate message
	ESP_LOGI(TAG, "addServiceMessageToQueue (%d) %s\n", serviceMessage.queueCount, cJSON_PrintUnformatted(message));
}

static void
serviceMessageTask (void *pvParameter)
{
	int cnt = 0;
  	while (1) {
		if (serviceMessage.read) {
			cnt = 0;
			if (serviceMessage.queueCount > 0) {
				cJSON_Delete(serviceMessage.message); // Delete old message
				serviceMessage.message = serviceMessage.messageQueue[serviceMessage.queueCount]; // Point to duplicated message
				ESP_LOGI(TAG, "serviceMessageTask (%d) %s\n", serviceMessage.queueCount, cJSON_PrintUnformatted(serviceMessage.message));
				serviceMessage.read = false;
				serviceMessage.queueCount--;
			}
		} else if (cnt > 10) {
			ESP_LOGE(TAG, "serviceMessage timeout reached.\n");
			// cJSON_Delete(serviceMessage.message); // Delete old message
			serviceMessage.read = true;
		} else {
			cnt++;
		}

    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  	}
}


void addServerMessageToQueue (char *message)
{
	serverMessage.queueCount++;
	strcpy(serverMessage.messageQueue[serverMessage.queueCount], message);
	ESP_LOGI(TAG, "addServerMessageToQueue (%d) %s\n", serverMessage.queueCount, message);
}

static void
serverMessageTask (void *pvParameter)
{
	int cnt = 0;
 	while (1) {
		if (!serverMessage.readyToSend) {
			if (serverMessage.queueCount > 0) {
				strcpy(serverMessage.message, serverMessage.messageQueue[serverMessage.queueCount]);
				serverMessage.readyToSend = true;
				serverMessage.queueCount--;
				ESP_LOGI(TAG, "serverMessageTask (%d)\n", serverMessage.queueCount);
			}
		}

    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  	}
}

void addClientMessageToQueue (char *message)
{
	clientMessage.queueCount++;
	strcpy(clientMessage.messageQueue[clientMessage.queueCount], message);
	ESP_LOGI(TAG, "addClientMessageToQueue (%d) %s\n", clientMessage.queueCount, message);

	addServerMessageToQueue(message); // Just adding all client message to server message que	
}

static void
clientMessageTask (void *pvParameter)
{
	int cnt = 0;
  	while (1) {
		if (!clientMessage.readyToSend) {
			if (clientMessage.queueCount > 0) {
				strcpy(clientMessage.message, clientMessage.messageQueue[clientMessage.queueCount]);
				clientMessage.readyToSend = true;
				clientMessage.queueCount--;
				ESP_LOGI(TAG, "clientMessageTask (%d)\n", clientMessage.queueCount);
			}
		}

    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  	}
}
