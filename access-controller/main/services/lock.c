#define LOCK_IO_1        A6
#define LOCK_IO_2        B6
#define NUM_OF_LOCKS 		 2

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
  uint8_t channel;
	bool isContact;
  bool isLocked;
  bool pulse;
	bool expired;
	bool enable;
	int delay;
	int count;
	bool alert;
	cJSON *payload;
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

int store_lock_state(cJSON * state)
{
  printf("Storing lock state: %s\n",cJSON_PrintUnformatted(state));
  store_char("lock", cJSON_PrintUnformatted(state));
  return 0;
}

void enableLock (int ch, bool val)
{
	for (int i=0; i < NUM_OF_LOCKS; i++)
		if (locks[i].channel == ch) locks[i].enable = val;
}

int load_lock_state_from_flash()
{
  char *state_str = get_char("lock");
  if (strcmp(state_str,"")==0) {
    printf("Lock state not found in flash.\n");
    return 1;
  }

  // Need JSON validation
  cJSON *lock_payload = cJSON_Parse(state_str);
  printf("Loaded lock state from flash. %s\n", state_str);
  return 0;
}

void _arm_lock (struct lock *lck, bool val)
{
  if (val) {
    set_mcp_io(lck->controlPin, ARM);
    printf("Lock %d armed.\n", lck->channel);
		start_lock_contact_timer(lck, false);
		if (lck->alert) beep(1);
  } else {
    set_mcp_io(lck->controlPin, DISARM);
    printf("Lock %d disarmed.\n", lck->channel);
		start_lock_contact_timer(lck, false);
		if (lck->alert) beep(2);
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
    locks[0].controlPin = LOCK_IO_1;
    locks[0].isLocked = true;
		locks[0].contactPin = LOCK_CONTACT_PIN_1;
		locks[0].enable = true;
		locks[0].alert = true;

    locks[1].channel = 2;
    locks[1].controlPin = LOCK_IO_2;
    locks[1].isLocked = true;
		locks[1].contactPin = LOCK_CONTACT_PIN_2;
		locks[1].enable = false;
		locks[1].alert = true;

		for (int i=0; i < NUM_OF_LOCKS; i++) {
			set_mcp_io_dir(locks[i].contactPin, MCP_INPUT);
			arm_lock(locks[i].channel, true, true);
		}
}

void handle_lock_message(cJSON * payload)
{
	if (payload == NULL) return;
	bool tmp;

  printf("lock payload: %s\n",cJSON_PrintUnformatted(payload));
	int ch=0;

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;
	} else {
		return;
	}

	if (cJSON_GetObjectItem(payload,"arm")) {
		if (cJSON_IsTrue(cJSON_GetObjectItem(payload,"arm"))) {
			arm_lock(ch, true, true);
		} else {
			arm_lock(ch, false, true);
		}
	}

	if (cJSON_GetObjectItem(payload,"enable")) {
		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
		enableLock(ch, tmp);
	}

	return;
}

static void
lock_service(void *pvParameter)
{
  // load_lock_state_from_flash();

  int cnt = 0;

  while (1) {
		for (int i=0; i < NUM_OF_LOCKS; i++)
			locks[i].isContact = !get_mcp_io(locks[i].contactPin);

		handle_lock_message(checkServiceMessage("lock"));

    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void lock_main()
{
  printf("Starting lock service.\n");
  lock_init();
  TaskHandle_t lock_service_task;

  xTaskCreate(&lock_service, "lock_service_task", 5000, NULL, 5, NULL);
  xTaskCreate(lock_contact_timer, "lock_contact_timer", 2048, NULL, 10, NULL);
}
