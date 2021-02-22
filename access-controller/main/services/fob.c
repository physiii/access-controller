#define FOB_IO_1         A5
#define FOB_IO_2         B5
#define NUM_OF_FOBS		 	 2

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
	int delay;
	int channel;
	cJSON *payload;
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
		printf("Re-arming lock from button %d service.\n", fb->channel);
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

int store_fob_state(cJSON * state)
{
  printf("Storing fob state: %s\n",cJSON_PrintUnformatted(state));
  store_char("fob", cJSON_PrintUnformatted(state));
  return 0;
}

int load_fob_state_from_flash()
{
  char *state_str = get_char("fob");
  if (strcmp(state_str,"")==0) {
    printf("Lock state not found in flash.\n");
    return 1;
  }

  // Need JSON validation
  cJSON *fob_payload = cJSON_Parse(state_str);
  printf("Loaded fob state from flash. %s\n", state_str);
  return 0;
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

	fb->isPressed = !get_mcp_io(fb->pin);

	if (fb->isPressed != fb->prevPress) {
		arm_lock(fb->channel, fb->isPressed, fb->alert);
		enableExit(fb->channel, fb->isPressed);
	}

	fb->prevPress = fb->isPressed;
}

void alertOnFob (int ch, bool val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (fobs[i].channel == ch) fobs[i].alert = val;
}

void handle_fob_message(cJSON * payload)
{
	if (payload == NULL) return;

	int ch=0;

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;
	} else {
		return;
	}

	if (cJSON_GetObjectItem(payload,"alert")) {
		if (cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"))) {
			alertOnFob(ch, true);
		} else {
			alertOnFob(ch, false);
		}
	}

	return;
}

static void
fob_service (void *pvParameter)
{
  // load_lock_state_from_flash();

  while (1) {
		for (int i=0; i < NUM_OF_FOBS; i++)
			check_fobs(&fobs[i]);

		handle_fob_message(checkServiceMessage("fob"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void fob_main()
{
  printf("Starting fob service.\n");

	fobs[0].pin = FOB_IO_1;
	fobs[0].delay = 4;
	fobs[0].channel = 1;
	fobs[0].enable = true;
	fobs[0].alert = true;

	fobs[1].pin = FOB_IO_2;
	fobs[1].delay = 4;
	fobs[1].channel = 2;
	fobs[1].enable = false;
	fobs[1].alert = true;

	set_mcp_io_dir(fobs[0].pin, MCP_INPUT);
	set_mcp_io_dir(fobs[1].pin, MCP_INPUT);

  xTaskCreate(fob_timer, "fob_timer", 2048, NULL, 10, NULL);
	xTaskCreate(fob_service, "fob_service", 2048, NULL, 10, NULL);
}
