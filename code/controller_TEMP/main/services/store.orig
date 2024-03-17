#include "storage.c"
#include <inttypes.h>

char store_service_message[1000];
bool store_service_message_ready = false;

struct Setting {
	char str[1000];
	uint32_t cnt;
	char key[50];
	cJSON *obj;
};

struct Setting setting;
// struct payloadState payload;

int storeSetting(char *key, cJSON *payload)
{
	// snprintf(setting.str, "%s", cJSON_PrintUnformatted(payload));
  	store_char(key, cJSON_PrintUnformatted(payload));
	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	return 0;
}

int restoreSetting (char *key) {
	strcpy(setting.str, get_char(key));
	printf("restoreSetting\t%s\n", setting.str);
	if (strcmp(setting.str, "")==0) return 1;

	// serviceMessage.message = cJSON_Parse(setting.str);
	cJSON *msg;
	msg = cJSON_Parse(setting.str);
	if (msg) addServiceMessageToQueue(msg);

	return 0;
}

void store_main()
{
  printf("starting store service\n");
	ESP_ERROR_CHECK(nvs_flash_init());
}
