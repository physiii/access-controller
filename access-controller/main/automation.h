
struct ServiceMessage
{
	cJSON *message;
	bool read;
};

struct ServiceMessage serviceMessage;

cJSON * checkServiceMessage(char *eventType)
{
	cJSON *payload = NULL;
	if (serviceMessage.read) return payload;
	payload = cJSON_GetObjectItem(serviceMessage.message,"payload");
	if (serviceMessage.message == NULL) return payload;
	if (!cJSON_GetObjectItem(serviceMessage.message,"eventType")) return payload;

	char type[50] = "";
	strcpy(type,cJSON_GetObjectItem(serviceMessage.message,"eventType")->valuestring);

	if (strcmp(type, eventType)) return payload;
	printf("checkServiceMessage\t%s\n", eventType);
	if (!cJSON_GetObjectItem(serviceMessage.message,"payload")) return payload;

	payload = cJSON_GetObjectItem(serviceMessage.message,"payload");

	serviceMessage.read = true;
	return payload;
}
