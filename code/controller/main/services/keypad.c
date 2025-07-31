#define KEYPAD_MCP_IO_1         A3
#define KEYPAD_MCP_IO_2         B3
#define NUM_OF_KEYPADS			    2

char keypad_service_message[2000];
bool keypad_service_message_ready = false;
cJSON * keypad_payload = NULL;

struct keypadButton
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

struct keypadButton keypads[NUM_OF_KEYPADS];

void start_keypad_timer (struct keypadButton *pad, bool val)
{
  if (val) {
    pad->expired = false;
    pad->count = 0;
  } else {
    pad->expired = true;
  }
}

void check_keypad_timer (struct keypadButton *pad)
{
  if (pad->count >= pad->delay && !pad->expired) {
		printf("Re-arming lock from pad %d service. Alert %d\n", pad->channel, pad->alert);
		arm_lock(pad->channel, true, pad->alert);
		pad->expired = true;
  } else pad->count++;
}

static void
keypad_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_KEYPADS; i++)
			check_keypad_timer(&keypads[i]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int storeKeypadSettings()
{
	for (uint8_t i=0; i < NUM_OF_KEYPADS; i++) {
		char type[25] = "";
		strcpy(type, keypads[i].type);
		sprintf(keypads[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": \"%s\", \"alert\": \"%s\", \"delay\": %d}}",
			type,
			i+1,
			(keypads[i].enable) ? "true" : "false",
			(keypads[i].alert) ? "true" : "false",
			keypads[i].delay);

		sprintf(keypads[i].key, "%s%d", type, i);
		storeSetting(keypads[i].key, cJSON_Parse(keypads[i].settings));
		printf("storeKeypadSettings\t%s\n", keypads[i].settings);
	}
  return 0;
}

int restoreKeypadSettings()
{
	for (uint8_t i=0; i < NUM_OF_KEYPADS; i++) {
		char type[25] = "";
		strcpy(type, keypads[i].type);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		sprintf(keypads[i].key, "%s%d", type, i);
		restoreSetting(keypads[i].key);
	}
	return 0;
}

int load_keypad_state_from_flash()
{
  char *state_str = get_char("keypad");
  if (strcmp(state_str,"")==0) {
    printf("Lock state not found in flash.\n");
    return 1;
  }

  // Need JSON validation
  cJSON *keypad_payload = cJSON_Parse(state_str);
  printf("Loaded keypad state from flash. %s\n", state_str);
  return 0;
}

int handle_keypad_property (char * prop)
{
  printf("keypad property: %s\n",prop);

	if (strcmp(prop,"keypad")==0) {
	}

	return 0;
}

void enableKeypad (int ch, bool val)
{
	for (int i=0; i < NUM_OF_KEYPADS; i++)
		if (keypads[i].channel == ch) keypads[i].enable = val;
}

void alertOnKeypad (int ch, bool val)
{
	printf("alertOnKeypad\tch: %d alert: %d.\n", ch, val);
	for (int i=0; i < NUM_OF_KEYPADS; i++)
		if (keypads[i].channel == ch) keypads[i].alert = val;
}

void setKeypadArmDelay (int ch, int val)
{
	for (int i=0; i < NUM_OF_KEYPADS; i++)
		if (keypads[i].channel == ch) keypads[i].delay = val;
}

void check_keypads (struct keypadButton *pad)
{
	if (!pad->enable) return;

	pad->isPressed = !get_io(pad->pin);

	if (pad->isPressed && !pad->prevPress) {
		printf("Disarming lock from pad %d service. Alert %d\n", pad->channel, pad->alert);
		arm_lock(pad->channel, false, pad->alert);
		start_keypad_timer(pad, true);
	}

	pad->prevPress = pad->isPressed;
}

void handle_keypad_message(cJSON * payload)
{
	int ch=0;
	bool tmp;

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendExitState();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		 if (cJSON_GetObjectItem(payload,"alert")) {
			 tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
			 alertOnKeypad(ch, tmp);
		 }

		 if (cJSON_GetObjectItem(payload,"enable")) {
			 tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
			 enableKeypad(ch, tmp);
		 }

		 if (cJSON_GetObjectItem(payload,"delay")) {
			 setKeypadArmDelay(ch, cJSON_GetObjectItem(payload,"delay")->valueint);
		 }

		 storeKeypadSettings();
	}

	return;
}

static void
keypad_service (void *pvParameter)
{
  // load_lock_state_from_flash();

  while (1) {
		for (int i=0; i < NUM_OF_KEYPADS; i++)
			check_keypads(&keypads[i]);

		                handle_keypad_message(checkServiceMessageByType("keypad"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void keypad_main()
{
  printf("Starting keypad service.\n");

	keypads[0].pin = USE_MCP23017 ? KEYPAD_MCP_IO_1 : KEYPAD_IO_1;
	keypads[0].delay = 4;
	keypads[0].channel = 1;
	keypads[0].alert = true;
	keypads[0].enable = true;
	strcpy(keypads[0].type, "keypad");

	keypads[1].pin = USE_MCP23017 ? KEYPAD_MCP_IO_2 : KEYPAD_IO_2;
	keypads[1].delay = 4;
	keypads[1].channel = 2;
	keypads[1].enable = true;
	keypads[1].alert = true;
	strcpy(keypads[1].type, "keypad");

	
	if (USE_MCP23017) {
		set_mcp_io_dir(keypads[0].pin, MCP_INPUT);
		set_mcp_io_dir(keypads[1].pin, MCP_INPUT);
	} else {
		gpio_set_direction(keypads[0].pin, GPIO_MODE_INPUT);
		gpio_set_direction(keypads[1].pin, GPIO_MODE_INPUT);
	}

  xTaskCreate(keypad_timer, "keypad_timer", 4096, NULL, 10, NULL);
	xTaskCreate(keypad_service, "keypad_service", 4096, NULL, 10, NULL);

	// restoreKeypadSettings();
}
