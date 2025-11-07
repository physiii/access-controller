#define MOTION_MCP_IO_1 A6  // Reverted back to correct pin
#define MOTION_MCP_IO_2 B6  // Reverted back to correct pin
#define NUM_OF_MOTIONS				  2

// MCP23017 constants and function declarations
#define MCP_OUTPUT 0
#define MCP_INPUT  1
void set_mcp_io_dir(uint8_t io, bool dir);

char motion_service_message[2000];
bool motion_service_message_ready = false;
cJSON * motion_payload = NULL;

struct motionButton
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

struct motionButton motions[NUM_OF_MOTIONS];

int storeMotionSettings()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char type[25] = "";
		strcpy(type, motions[i].type);
		sprintf(motions[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
			type,
			i+1,
			(motions[i].enable) ? "true" : "false",
			(motions[i].alert) ? "true" : "false",
			motions[i].delay);

		sprintf(motions[i].key, "%s%d", type, i);
		storeSetting(motions[i].key, cJSON_Parse(motions[i].settings));
		// printf("storeMotionSettings\t%s\n", motions[i].settings);
	}
  return 0;
}

int restoreMotionSettings()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char type[25] = "";
		strcpy(type, motions[i].type);
		sprintf(motions[i].key, "%s%d", type, i);
		restoreSetting(motions[i].key);
    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	}
	return 0;
}

void
sendMotionEventToServer()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char state_str[300];
		char msg[600];

		snprintf(state_str, sizeof(state_str), "{\"presence\":%s, \"exit\":false, \"keypad\":false, \"uptime\":1}", motions[i].isPressed ? "true" : "false");
		snprintf(msg, sizeof(msg),"{\"event_type\":\"load\", \"payload\":{\"services\":"
			"[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

		addServerMessageToQueue(msg);
		printf("sendMotionEventToServer: %s\n", msg);
	}
}

void sendMotionEventToClient(int channel, bool state) {
    for (int i=0; i<NUM_OF_MOTIONS; i++) {
        if (motions[i].channel == channel) {
            if (strlen(motions[i].settings) > 2) {
                cJSON *json_msg = cJSON_Parse(motions[i].settings);
                addClientMessageToQueue(json_msg);
                cJSON_Delete(json_msg);
            }
        }
    }
}

void enableMotion (int ch, bool val)
{
	for (int i=0; i < NUM_OF_MOTIONS; i++)
		if (motions[i].channel == ch) motions[i].enable = val;
}

void alertOnMotion (int ch, bool val)
{
	for (int i=0; i < NUM_OF_MOTIONS; i++)
		if (motions[i].channel == ch) motions[i].alert = val;
}

void start_motion_timer (struct motionButton *mot, bool val)
{
  if (val) {
    mot->expired = false;
    mot->count = 0;
  } else {
    mot->expired = true;
  }
}

void check_motion (struct motionButton *mot)
{
	if (!mot->enable) return;

	mot->isPressed = !get_mcp_io(mot->pin);

	if (mot->isPressed && !mot->prevPress) {
		ESP_LOGI(TAG, "Motion detected on channel %d - disarming lock", mot->channel);
        lock_set_action_source("motion");
		arm_lock(mot->channel, false, mot->alert);
		start_motion_timer(mot, true);
	}

	mot->prevPress = mot->isPressed;
}

void handle_motion_message(cJSON * payload)
{
	int ch=0;
	bool tmp = 0;
	char state[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendMotionEventToClient(motions[0].channel, motions[0].isPressed);
		sendMotionEventToServer();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		 if (cJSON_GetObjectItem(payload,"alert")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
	 		alertOnMotion(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"enable")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
	 		enableMotion(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"delay")) {
	 		setArmDelay(ch, cJSON_GetObjectItem(payload,"delay")->valueint);
	 	}
		storeMotionSettings();
	}

	cJSON_Delete(payload);
}

void motion_timer_func(struct motionButton *mot)
{
	if (mot->count >= mot->delay && !mot->expired) {
		ESP_LOGI(TAG, "Re-arming lock from motion %d service.", mot->channel);
        lock_set_action_source("motion_auto");
		arm_lock(mot->channel, true, mot->alert);
		mot->expired = true;
	} else {
		mot->count++;
	}
}

static void
motion_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_MOTIONS; i++)
			motion_timer_func(&motions[i]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void
motion_service (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_MOTIONS; i++) {
			check_motion(&motions[i]);
		}

		handle_motion_message(checkServiceMessageByType("motion"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void motion_main()
{
  ESP_LOGI(TAG, "Starting motion service.");

	motions[0].pin = MOTION_MCP_IO_1;
	motions[0].delay = 4;
	motions[0].channel = 1;
	motions[0].alert = true;
	motions[0].enable = true;
	strcpy(motions[0].type, "motion");

	motions[1].pin = MOTION_MCP_IO_2;
	motions[1].delay = 4;
	motions[1].channel = 2;
	motions[1].alert = true;
	motions[1].enable = true;
	strcpy(motions[1].type, "motion");

	restoreMotionSettings();

	// Configure motion pins as inputs
	if (USE_MCP23017) {
		set_mcp_io_dir(motions[0].pin, MCP_INPUT);
		set_mcp_io_dir(motions[1].pin, MCP_INPUT);
		ESP_LOGI(TAG, "Motion pins configured as inputs: pin %d, pin %d", motions[0].pin, motions[1].pin);
	} else {
		gpio_set_direction(motions[0].pin, GPIO_MODE_INPUT);
		gpio_set_direction(motions[1].pin, GPIO_MODE_INPUT);
	}

  xTaskCreate(motion_timer, "motion_timer", 4096, NULL, 10, NULL);
	xTaskCreate(motion_service, "motion_service", 5000, NULL, 10, NULL);
}
