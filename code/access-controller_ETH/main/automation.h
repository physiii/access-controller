#define SERVICE_LOOP 100
#define SERVICE_LOOP_SHORT 10
#define USE_MCP23017	   1

struct ServiceMessage
{
	cJSON *message;
	cJSON messageQueue[100];
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

struct ServiceMessage serviceMessage;
struct ClientMessage clientMessage;

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
	serviceMessage.messageQueue[serviceMessage.queueCount] = *message;
	printf("addServiceMessageToQueue (%d) %s\n", serviceMessage.queueCount, cJSON_PrintUnformatted(message));
}

static void
serviceMessageTask (void *pvParameter)
{
	int cnt = 0;
  while (1) {
		if (serviceMessage.read) {
			cnt = 0;
			if (serviceMessage.queueCount > 0) {
				serviceMessage.message = &serviceMessage.messageQueue[serviceMessage.queueCount];
				printf("serviceMessageTask (%d) %s\n", serviceMessage.queueCount, cJSON_PrintUnformatted(serviceMessage.message));
				serviceMessage.read = false;
				serviceMessage.queueCount--;
			}
		} else if (cnt > 10) {
			printf("serviceMessage timeout reached.\n");
			serviceMessage.read = true;
		} else {
			cnt++;
		}

    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void addClientMessageToQueue (char *message)
{
	clientMessage.queueCount++;
	strcpy(clientMessage.messageQueue[clientMessage.queueCount], message);
	printf("addClientMessageToQueue (%d) %s\n", clientMessage.queueCount, message);
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
				printf("clientMessageTask (%d)\n", clientMessage.queueCount);
			}
		}

    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}
