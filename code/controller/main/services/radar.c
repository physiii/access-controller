#define RADAR_BUTTON_MCP_IO_1         A3
#define RADAR_BUTTON_MCP_IO_2         B3
#define NUM_OF_RADARS				  1

char radar_service_message[2000];
bool radar_service_message_ready = false;
cJSON * radar_payload = NULL;

struct radarButton
{
  int pin;
	bool alert;
	bool isPressed;
	bool prevPress;
	int count;
	bool expired;
	bool enable;
	int delay;
	int channel;
	char settings[1000];
	char key[50];
	char type[40];
	cJSON *payload;
};

struct radarButton radars[NUM_OF_RADARS];

int storeRadarSettings()
{
	for (uint8_t i=0; i < NUM_OF_RADARS; i++) {
		char type[25] = "";
		strcpy(type, radars[i].type);
		sprintf(radars[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
			type,
			i+1,
			(radars[i].enable) ? "true" : "false",
			(radars[i].alert) ? "true" : "false",
			radars[i].delay);

		sprintf(radars[i].key, "%s%d", type, i);
		storeSetting(radars[i].key, cJSON_Parse(radars[i].settings));
		// printf("storeRadarSettings\t%s\n", radars[i].settings);
	}
  return 0;
}


int restoreRadarSettings()
{
	for (uint8_t i=0; i < NUM_OF_RADARS; i++) {
		char type[25] = "";
		strcpy(type, radars[i].type);
		sprintf(radars[i].key, "%s%d", type, i);
		restoreSetting(radars[i].key);
    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	}
	return 0;
}

void
sendRadarEventToServer()
{
	for (uint8_t i=0; i < NUM_OF_RADARS; i++) {
		char state_str[300];
		char msg[600];

		snprintf(state_str, sizeof(state_str), "{\"presence\":%s, \"exit\":false, \"keypad\":false, \"uptime\":1}", radars[i].isPressed ? "true" : "false");
		snprintf(msg, sizeof(msg),"{\"event_type\":\"load\", \"payload\":{\"services\":"
			"[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

		addServerMessageToQueue(msg);
		printf("sendRadarEventToServer: %s\n", msg);
	}
}

int
sendRadarEventToClient()
{
	for (uint8_t i=0; i < NUM_OF_RADARS; i++) {
		char type[25] = "";
		strcpy(type, radars[i].type);
		sprintf(radars[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d, \"presence\": %s}}",
			type,
			i+1,
			(radars[i].enable) ? "true" : "false",
			(radars[i].alert) ? "true" : "false",
			radars[i].delay,
			(radars[i].isPressed) ? "true" : "false");

		addClientMessageToQueue(radars[i].settings);
		printf("sendRadarEventToClient: %s\n", radars[i].settings);
	}
	return 0;
}

void enableRadar (int ch, bool val)
{
	for (int i=0; i < NUM_OF_RADARS; i++)
		if (radars[i].channel == ch) radars[i].enable = val;
}

void alertOnRadar (int ch, bool val)
{
	for (int i=0; i < NUM_OF_RADARS; i++)
		if (radars[i].channel == ch) radars[i].alert = val;
}

void check_radar (struct radarButton *ext)
{
	if (!ext->enable) return;

	ext->isPressed = get_io(ext->pin);

	if (ext->isPressed != ext->prevPress) {
		sendRadarEventToClient();
		sendRadarEventToServer();
	}

	ext->prevPress = ext->isPressed;
}

void handle_radar_message(cJSON * payload)
{
	int ch=0;
	bool tmp = 0;
	char state[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendRadarEventToClient();
		sendRadarEventToServer();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		 if (cJSON_GetObjectItem(payload,"alert")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
	 		alertOnRadar(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"enable")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
	 		enableRadar(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"delay")) {
	 		setArmDelay(ch, cJSON_GetObjectItem(payload,"delay")->valueint);
	 	}
		storeRadarSettings();
	}
}

static void
radar_service (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_RADARS; i++)
			check_radar(&radars[i]);

		handle_radar_message(checkServiceMessage("radar"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void radar_main()
{
  printf("Starting radar service.\n");

	radars[0].pin = USE_MCP23017 ? RADAR_BUTTON_MCP_IO_1 : RADAR_BUTTON_IO_1;
	radars[0].delay = 4;
	radars[0].channel = 1;
	radars[0].alert = false;
	radars[0].enable = true;
	strcpy(radars[0].type, "radar");

	restoreRadarSettings();

	// if (USE_MCP23017) {
	// 	set_mcp_io_dir(radars[0].pin, MCP_INPUT);
	// 	set_mcp_io_dir(radars[1].pin, MCP_INPUT);
	// } else {
	// 	gpio_set_direction(radars[0].pin, GPIO_MODE_INPUT);
	// 	gpio_set_direction(radars[1].pin, GPIO_MODE_INPUT);
	// }

	// xTaskCreate(radar_service, "radar_service", 5000, NULL, 10, NULL);
}
