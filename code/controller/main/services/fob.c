#define FOB_MCP_IO_1         A7
#define FOB_MCP_IO_2         B7
#define NUM_OF_FOBS			    2
#define MOMENTARY		  1

char fob_service_message[2000];
bool fob_service_message_ready = false;
cJSON * fob_payload = NULL;
bool FOB_ALERT = true;

struct fob
{
  	int pin;
	bool alert;
	bool isPressed;
	bool prevPress;
	int count;
	bool expired;
	bool enable;
	bool latch;  // New field for latch mode (false = momentary, true = latch)
	int delay;
	int channel;
	cJSON *payload;
	char settings[1000];
	char key[50];
	char type[40];
};

struct fob fobs[NUM_OF_FOBS];

void start_fob_timer (struct fob *fb, bool val)
{
  if (val) {
    fb->expired = false;
    fb->count = 0;
  } else {
    fb->expired = true;
  }
}

void check_fob_timer (struct fob *fb)
{
  if (fb->count >= fb->delay && !fb->expired) {
		ESP_LOGI(TAG, "Re-arming lock from fob %d service.", fb->channel);
		arm_lock(fb->channel, true, fb->alert);
		fb->expired = true;
  } else fb->count++;
}

static void
fob_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_FOBS; i++)
			check_fob_timer(&fobs[i]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int handle_fob_property (char * prop)
{
  printf("fob property: %s\n",prop);

	if (strcmp(prop,"fob")==0) {
	}

	return 0;
}

void check_fobs (struct fob *fb)
{
	if (!fb->enable) return;

	fb->isPressed = get_mcp_io(fb->pin);

	if (fb->latch && fb->isPressed != fb->prevPress) {
		// Latch mode: FOB state directly controls lock state
		ESP_LOGI(TAG, "Fob %d state changed to %s (latch mode)", fb->channel, fb->isPressed ? "activated" : "deactivated");
		arm_lock(fb->channel, fb->isPressed, fb->alert);
		enableExit(fb->channel, fb->isPressed);
		enableKeypad(fb->channel, fb->isPressed);
	} else if (!fb->latch && !fb->isPressed && fb->prevPress) {
		// Momentary mode: FOB release triggers unlock and timer
		ESP_LOGI(TAG, "Fob %d released (momentary mode) - disarming lock", fb->channel);
		arm_lock(fb->channel, false, fb->alert);
		start_fob_timer(fb, true);
	}

	fb->prevPress = fb->isPressed;
}

void alertOnFob (int ch, bool val)
{
	for (int i=0; i < NUM_OF_FOBS; i++)
		if (fobs[i].channel == ch) fobs[i].alert = val;
}


int storeFobSettings()
{

	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		char type[25] = "";
		strcpy(type, fobs[i].type);
		sprintf(fobs[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"latch\": %s}}",
			type,
			i+1,
			(fobs[i].enable) ? "true" : "false",
			(fobs[i].alert) ? "true" : "false",
			(fobs[i].latch) ? "true" : "false");

		sprintf(fobs[i].key, "%s%d", type, i);
		storeSetting(fobs[i].key, cJSON_Parse(fobs[i].settings));
		// printf("storeFobSettings\t%s\n", fobs[i].settings);
	}
  return 0;
}

int restoreFobSettings()
{
	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		char type[25] = "";
		strcpy(type, fobs[i].type);
		sprintf(fobs[i].key, "%s%d", type, i);
		restoreSetting(fobs[i].key);
	}
	return 0;
}

void sendFobState(void) {
    for (int i=0; i < NUM_OF_FOBS; i++) {
        if (strlen(fobs[i].settings) > 2) {
            cJSON *json_msg = cJSON_Parse(fobs[i].settings);
            addClientMessageToQueue(json_msg);
            cJSON_Delete(json_msg);
        }
    }
}

cJSON *fob_state_snapshot(void) {
    cJSON *array = cJSON_CreateArray();
    if (!array) {
        return NULL;
    }

    for (int i = 0; i < NUM_OF_FOBS; i++) {
        cJSON *entry = cJSON_CreateObject();
        if (!entry) {
            continue;
        }
        cJSON_AddNumberToObject(entry, "channel", fobs[i].channel);
        cJSON_AddBoolToObject(entry, "enable", fobs[i].enable);
        cJSON_AddBoolToObject(entry, "alert", fobs[i].alert);
        cJSON_AddBoolToObject(entry, "latch", fobs[i].latch);
        cJSON_AddItemToArray(array, entry);
    }

    return array;
}

void handle_fob_message(cJSON * payload)
{
	if (payload == NULL) return;

	int ch = 0;
	bool val = false;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendFobState();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		if (cJSON_GetObjectItem(payload,"enable")) {
			val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
			fobs[ch - 1].enable = val;
		}

		if (cJSON_GetObjectItem(payload,"alert")) {
			val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
			fobs[ch - 1].alert = val;
		}

		if (cJSON_GetObjectItem(payload,"latch")) {
			val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"latch"));
			fobs[ch - 1].latch = val;
		}
		storeFobSettings();
	}
}

static void
fob_service (void *pvParameter)
{
  // load_fob_state_from_flash();

  while (1) {
		for (int i=0; i < NUM_OF_FOBS; i++)
			check_fobs(&fobs[i]);

		                handle_fob_message(checkServiceMessageByType("fob"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void fob_main()
{
  printf("Starting fob service.\n");

	fobs[0].pin = FOB_MCP_IO_1;
	fobs[0].delay = 4;
	fobs[0].channel = 1;
	fobs[0].enable = false;
	fobs[0].alert = false;
	fobs[0].latch = false;  // Default to momentary mode
	strcpy(fobs[0].type, "fob");

	fobs[1].pin = FOB_MCP_IO_2;
	fobs[1].delay = 4;
	fobs[1].channel = 2;
	fobs[1].enable = false;
	fobs[1].alert = false;
	fobs[1].latch = false;  // Default to momentary mode
	strcpy(fobs[1].type, "fob");

	if (USE_MCP23017) {
		set_mcp_io_dir(fobs[0].pin, MCP_INPUT);
		set_mcp_io_dir(fobs[1].pin, MCP_INPUT);
	} else {
		gpio_set_direction(fobs[0].pin, GPIO_MODE_INPUT);
		gpio_set_direction(fobs[1].pin, GPIO_MODE_INPUT);
	}

  	xTaskCreate(fob_timer, "fob_timer", 4096, NULL, 10, NULL);
	xTaskCreate(fob_service, "fob_service", 5000, NULL, 10, NULL);

	restoreFobSettings();
}
