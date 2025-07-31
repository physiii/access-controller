#define EXIT_BUTTON_MCP_IO_1         A3
#define EXIT_BUTTON_MCP_IO_2         B3
#define NUM_OF_EXITS						 2

char exit_service_message[2000];
bool exit_service_message_ready = false;
cJSON * exit_payload = NULL;

struct exitButton
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

struct exitButton exits[NUM_OF_EXITS];

void start_exit_timer (struct exitButton *ext, bool val)
{
  if (val) {
    ext->expired = false;
    ext->count = 0;
  } else {
    ext->expired = true;
  }
}

void check_exit_timer (struct exitButton *ext)
{
  if (ext->count >= ext->delay && !ext->expired) {
		printf("Re-arming lock from button %d service.\n", ext->channel);
		arm_lock(ext->channel, true, ext->alert);
		ext->expired = true;
  } else ext->count++;
}

static void
exit_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_EXITS; i++)
			check_exit_timer(&exits[i]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int storeExitSettings()
{

	for (uint8_t i=0; i < NUM_OF_EXITS; i++) {
		char type[25] = "";
		strcpy(type, exits[i].type);
		sprintf(exits[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
			type,
			i+1,
			(exits[i].enable) ? "true" : "false",
			(exits[i].alert) ? "true" : "false",
			exits[i].delay);

		sprintf(exits[i].key, "%s%d", type, i);
		storeSetting(exits[i].key, cJSON_Parse(exits[i].settings));
		// printf("storeExitSettings\t%s\n", exits[i].settings);
	}
  return 0;
}


int restoreExitSettings()
{
	for (uint8_t i=0; i < NUM_OF_EXITS; i++) {
		char type[25] = "";
		strcpy(type, exits[i].type);
		sprintf(exits[i].key, "%s%d", type, i);
		restoreSetting(exits[i].key);
    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	}
	return 0;
}

void sendExitState(void) {
    for (int i=0; i < NUM_OF_EXITS; i++) {
        if (strlen(exits[i].settings) > 2) {
            cJSON *json_msg = cJSON_Parse(exits[i].settings);
            addClientMessageToQueue(json_msg);
            cJSON_Delete(json_msg);
        }
    }
}


int handle_exit_property (char * prop)
{
  printf("exit property: %s\n",prop);

	if (strcmp(prop,"exit")==0) {
	}

	return 0;
}

void enableExit (int ch, bool val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].enable = val;
}

void alertOnExit (int ch, bool val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].alert = val;
}

void setArmDelay (int ch, int val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].delay = val;
}

void check_exit (struct exitButton *ext)
{
	if (!ext->enable) return;

	ext->isPressed = !get_io(ext->pin);

	if (ext->isPressed && !ext->prevPress) {
		arm_lock(ext->channel, false, ext->alert);
		start_exit_timer(ext, true);
	}

	ext->prevPress = ext->isPressed;
}

void handle_exit_message(cJSON * payload)
{
	int ch=0;
	bool tmp = 0;
	char state[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendExitState();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		 if (cJSON_GetObjectItem(payload,"alert")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
	 		alertOnExit(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"enable")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
	 		enableExit(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"delay")) {
	 		setArmDelay(ch, cJSON_GetObjectItem(payload,"delay")->valueint);
	 	}
		storeExitSettings();
	}
}

static void
exit_service (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_EXITS; i++)
			check_exit(&exits[i]);

		handle_exit_message(checkServiceMessage("exit"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void exit_main()
{
  printf("Starting exit service.\n");

	exits[0].pin = USE_MCP23017 ? EXIT_BUTTON_MCP_IO_1 : EXIT_BUTTON_IO_1;
	exits[0].delay = 4;
	exits[0].channel = 1;
	exits[0].alert = false;
	exits[0].enable = false;
	strcpy(exits[0].type, "exit");

	exits[1].pin = USE_MCP23017 ? EXIT_BUTTON_MCP_IO_2 : EXIT_BUTTON_IO_2;
	exits[1].delay = 4;
	exits[1].channel = 2;
	exits[1].enable = false;
	exits[1].alert = false;
	strcpy(exits[1].type, "exit");

	restoreExitSettings();

	if (USE_MCP23017) {
		set_mcp_io_dir(exits[0].pin, MCP_INPUT);
		set_mcp_io_dir(exits[1].pin, MCP_INPUT);
	} else {
		gpio_set_direction(exits[0].pin, GPIO_MODE_INPUT);
		gpio_set_direction(exits[1].pin, GPIO_MODE_INPUT);
	}

  	xTaskCreate(exit_timer, "exit_timer", 2048, NULL, 10, NULL);
	xTaskCreate(exit_service, "exit_service", 5000, NULL, 10, NULL);
}
