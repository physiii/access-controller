#define LOCK_MCP_IO_1        A6
#define LOCK_MCP_IO_2        B6
#define NUM_OF_LOCKS 				 2

const uint8_t LOCK_CONTACT_PIN_1 = B1;
const uint8_t LOCK_CONTACT_PIN_2 = A7;

bool ARM = false;
bool DISARM = true;
bool ALERT = true;
char lock_service_message[2000];
bool lock_service_message_ready = false;
int relock_delay = 2 * 1000;
int button_disabled = false;
cJSON * lock_payload = NULL;
int LOCK_DEBOUNCE_DELAY = 5;
bool lock_contact_timer_1_expired = true;
int lock_count = 0;

struct lock
{
  uint8_t controlPin;
	uint8_t contactPin;
	uint8_t openPin;
  uint8_t channel;
	bool isContact;
  bool isLocked;
  bool pulse;
	bool expired;
	bool enable;
	bool enableContactAlert;
	bool arm;
	int delay;
	int count;
	bool alert;
	cJSON *payload;
	char settings[1000];
	char key[50];
	char type[40];
};

struct lock locks[NUM_OF_LOCKS];

void
start_lock_contact_timer(struct lock *lck, bool val)
{
  if (val) {
    lck->expired = !val;
    lck->count = 0;
  } else {
    lck->expired = val;
  }
}

void check_lock_contact_timer (struct lock *lck)
{
	if (lck >= lck->delay
		&& lck->enableContactAlert
		&& !lck->isContact
		&& lck->isLocked
		&& lck->enable
		&& !lck->expired) {
		printf("Contact delay expired for door %d, sounding alert.\n", lck->channel);
		longBeep(1);
	} else lck->count++;
}

static void
lock_contact_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_LOCKS; i++)
			check_lock_contact_timer(&locks[i]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void createLockServiceMessage(bool value)
{
  char value_str[10];
  if (value) {
    strcpy(value_str,"true");
  } else {
    strcpy(value_str,"false");
  }

  snprintf(lock_service_message,sizeof(lock_service_message),""
  "{\"event_type\":\"service/state\","
  " \"payload\":{\"service_id\":\"locks[0]\",\"state\":{\"locked\":%s}}}"
  , value_str);
  lock_service_message_ready = true;
}

void enableLock (int ch, bool val)
{
	for (int i=0; i < NUM_OF_LOCKS; i++)
		if (locks[i].channel == ch) locks[i].enable = val;
}

void _arm_lock (struct lock *lck, bool val)
{
  if (val) {
		set_io(lck->controlPin, ARM);
		start_lock_contact_timer(lck, false);

		if (lck->alert) {
			beep(1);
			set_io(lck->openPin, ARM);
		}

    printf("Lock %d armed.\n", lck->channel);
  } else {
	  set_io(lck->controlPin, DISARM);
		start_lock_contact_timer(lck, false);

		if (lck->alert) {
			beep(2);
			set_io(lck->openPin, DISARM);
		}

		printf("Lock %d disarmed.\n", lck->channel);
  }

  lck->isLocked = val;
}

void arm_lock (int channel, bool val, bool alert)
{
	for (int i=0; i < NUM_OF_LOCKS; i++) {
		locks[i].alert = alert;
		if (channel == locks[i].channel) _arm_lock(&locks[i], val);
	}
}

void lock_init()
{
    locks[0].channel = 1;
    locks[0].controlPin = USE_MCP23017 ? LOCK_MCP_IO_1 : LOCK_IO_1;
    locks[0].isLocked = true;
		locks[0].contactPin = USE_MCP23017 ? LOCK_CONTACT_PIN_1 : CONTACT_IO_1;
		locks[0].openPin = OPEN_IO_1;
		locks[0].enable = true;
		locks[0].alert = true;
		locks[0].enableContactAlert = false;
		strcpy(locks[0].type, "lock");

    locks[1].channel = 2;
    locks[1].controlPin = USE_MCP23017 ? LOCK_MCP_IO_2 : LOCK_IO_2;
    locks[1].isLocked = true;
		locks[1].contactPin = USE_MCP23017 ? LOCK_CONTACT_PIN_2 : CONTACT_IO_2;
		locks[1].openPin = OPEN_IO_2;
		locks[1].enable = true;
		locks[1].alert = true;
		locks[1].enableContactAlert = false;
		strcpy(locks[1].type, "lock");

		for (int i=0; i < NUM_OF_LOCKS; i++) {
			if (USE_MCP23017) {
				set_mcp_io_dir(locks[i].contactPin, MCP_INPUT);
			}
			// arm_lock(locks[i].channel, true, true);
		}
}

int storeLockSettings()
{

	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		sprintf(locks[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"arm\": %s, \"enableContactAlert\": %s}}",
			locks[i].type,
			i+1,
			(locks[i].enable) ? "true" : "false",
			(locks[i].isLocked) ? "true" : "false",
			(locks[i].enableContactAlert) ? "true" : "false");

		sprintf(locks[i].key, "%s%d", locks[i].type, i);
		storeSetting(locks[i].key, cJSON_Parse(locks[i].settings));
		printf("storeLockSettings\t%s\n", locks[i].settings);
	}
  return 0;
}

int restoreLockSettings()
{
	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		sprintf(locks[i].key, "%s%d", locks[i].type, i);
		restoreSetting(locks[i].key);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	return 0;
}

int sendLockState()
{

	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		sprintf(locks[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"arm\": %s, \"enableContactAlert\": %s}}",
			locks[i].type,
			i+1,
			(locks[i].enable) ? "true" : "false",
			(locks[i].isLocked) ? "true" : "false",
			(locks[i].enableContactAlert) ? "true" : "false");

		addClientMessageToQueue(locks[i].settings);
		// printf("sendLockSettings: %s\n", locks[i].settings);
	}
  return 0;
}

void handle_lock_message(cJSON * payload)
{
	int ch=0;
	bool val;

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendLockState();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
	 	ch = cJSON_GetObjectItem(payload,"channel")->valueint;

	 	if (cJSON_GetObjectItem(payload,"arm")) {
	 		val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"arm"));
	 		arm_lock(ch, val, true);
	 	}

	 	if (cJSON_GetObjectItem(payload,"enable")) {
	 		val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
	 		enableLock(ch, val);
	 	}

		if (cJSON_GetObjectItem(payload,"enableContactAlert")) {
	 		val = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enableContactAlert"));
			locks[ch - 1].enableContactAlert = val;
	 	}

		// printf("lock payload: %s\n",cJSON_PrintUnformatted(payload));
		storeLockSettings();
	}
	return;
}

static void
lock_service(void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_LOCKS; i++) {
			locks[i].isContact = !get_io(locks[i].contactPin);
			// printf("Lock %d |\tContact (%d): %d\n", i, locks[i].contactPin, locks[i].isContact);
		}

		handle_lock_message(checkServiceMessage("lock"));

    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void lock_main()
{
  printf("Starting lock service.\n");
  TaskHandle_t lock_service_task;

	lock_init();
	restoreLockSettings();

  xTaskCreate(&lock_service, "lock_service_task", 5000, NULL, 5, NULL);
  xTaskCreate(lock_contact_timer, "lock_contact_timer", 2048, NULL, 10, NULL);
}
