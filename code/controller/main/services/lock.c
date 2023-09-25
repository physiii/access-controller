#define LOCK_MCP_IO_1       A0
#define LOCK_MCP_IO_2       B0
#define NUM_OF_LOCKS        2

const uint8_t LOCK_CONTACT_PIN_1 = B1;
const uint8_t LOCK_CONTACT_PIN_2 = B1;

bool ARM = false;
bool ALERT = true;
char lock_service_message[1000];
bool lock_service_message_ready = false;
int relock_delay = 2 * 1000;
int button_disabled = false;
cJSON *lock_payload = NULL;
int LOCK_DEBOUNCE_DELAY = 5;
bool lock_contact_timer_1_expired = true;
int lock_count = 0;

typedef struct {
    uint8_t controlPin;
    uint8_t contactPin;
    uint8_t openPin;
    uint8_t channel;
    bool isContact;
    bool isLocked;
    bool pulse;
    bool polarity;
    bool expired;
    bool enable;
    bool enableContactAlert;
    bool arm;
    int delay;
    int count;
    bool alert;
    cJSON *payload;
    char settings[200];
    char key[50];
    char type[40];
} Lock;

Lock locks[NUM_OF_LOCKS];

void start_lock_contact_timer(Lock *lck, bool val) {
    lck->expired = !val;
    if (!val) {
        lck->count = 0;
    }
}

void check_lock_contact_timer(Lock *lck) {
    if (lck->count >= lck->delay && lck->enableContactAlert && !lck->isContact && lck->isLocked && lck->enable && !lck->expired) {
        ESP_LOGI(TAG, "Contact delay expired for door %d, sounding alert.", lck->channel);
        longBeep(1);
    } else {
        lck->count++;
    }
}

static void lock_contact_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_LOCKS; i++)
            check_lock_contact_timer(&locks[i]);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void createLockServiceMessage(bool value) {
    snprintf(lock_service_message, sizeof(lock_service_message),
             "{\"event_type\":\"service/state\",\"payload\":{\"service_id\":\"locks[0]\",\"state\":{\"locked\":%s}}}",
             value ? "true" : "false");
    lock_service_message_ready = true;
}

void enableLock(int ch, bool val) {
    for (int i = 0; i < NUM_OF_LOCKS; i++)
        if (locks[i].channel == ch) locks[i].enable = val;
}

void arm_lock(int channel, bool arm, bool alert) {
    int ch = channel - 1;
    if (!locks[ch].enable) return;
    locks[ch].alert = alert;

    set_io(locks[ch].controlPin, arm);
    start_lock_contact_timer(&locks[ch], false);
    if (locks[ch].alert) {
        beep(1);
        beep_keypad(1, locks[ch].channel);
    }
    locks[ch].isLocked = arm;
}

void lock_init() {
    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        locks[i].channel = i + 1;
        locks[i].controlPin = (i == 0) ? (USE_MCP23017 ? LOCK_MCP_IO_1 : LOCK_IO_1) : (USE_MCP23017 ? LOCK_MCP_IO_2 : LOCK_IO_2);
        locks[i].isLocked = true;
        locks[i].contactPin = (i == 0) ? (USE_MCP23017 ? LOCK_CONTACT_PIN_1 : CONTACT_IO_1) : (USE_MCP23017 ? LOCK_CONTACT_PIN_2 : CONTACT_IO_2);
        locks[i].openPin = (i == 0) ? OPEN_IO_1 : OPEN_IO_2;
        locks[i].enable = true;
        locks[i].delay = 4;
        locks[i].alert = true;
        locks[i].enableContactAlert = false;
        strcpy(locks[i].type, "lock");
    }
}

int storeLockSettings()
{
	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		char type[25] = "";
		strcpy(type, locks[i].type);

		sprintf(locks[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"arm\": %s, \"enableContactAlert\": %s}}",
			type,
			i+1,
			(locks[i].enable) ? "true" : "false",
			(locks[i].isLocked) ? "true" : "false",
			(locks[i].enableContactAlert) ? "true" : "false");

		sprintf(locks[i].key, "%s%d", type, i);
		storeSetting(locks[i].key, cJSON_Parse(locks[i].settings));
	}
	return 0;
}

int restoreLockSettings()
{
	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		char type[25] = "";
		strcpy(type, locks[i].type);
		sprintf(locks[i].key, "%s%d", type, i);
		restoreSetting(locks[i].key);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	return 0;
}

int sendLockState()
{

	for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
		char type[25] = "";
		strcpy(type, locks[i].type);
		sprintf(locks[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"arm\": %s, \"enableContactAlert\": %s}}",
			type,
			i+1,
			(locks[i].enable) ? "true" : "false",
			(locks[i].isLocked) ? "true" : "false",
			(locks[i].enableContactAlert) ? "true" : "false");

		addClientMessageToQueue(locks[i].settings);
	}
  return 0;
}

void handle_lock_message(cJSON * payload) {
    int ch=0;
    bool val;
    char property[250];

    if (payload == NULL) return;

	cJSON *channel_item = cJSON_GetObjectItem(payload, "channel");
	cJSON *enable_item = cJSON_GetObjectItem(payload, "enable");
	cJSON *enableContactAlert_item = cJSON_GetObjectItem(payload, "enableContactAlert");
	cJSON *arm_item = cJSON_GetObjectItem(payload, "arm");
	cJSON *property_item = cJSON_GetObjectItem(payload,"property");

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendLockState();
	}

	if (channel_item) {
	 	ch = channel_item->valueint;

	 	if (arm_item) {
	 		val = arm_item->type == cJSON_True;
	 		arm_lock(ch, val, true);
	 	}

		if (enable_item) {
			if (enable_item->type == cJSON_False || enable_item->type == cJSON_True) {
				val = cJSON_IsTrue(enable_item);
				enableLock(ch, val);
			}
		}

		if (enableContactAlert_item) {
	 		val = enableContactAlert_item->type == cJSON_True;
			locks[ch - 1].enableContactAlert = val;
	 	}

		storeLockSettings();
	}

	if (property_item) {
		if (property_item->valuestring == NULL) {
			cJSON_Delete(payload);
			return;
		}

		sprintf(property, "%s", property_item->valuestring);
		if (strcmp(property, "pulseLock") == 0) {

    		char log_msg[1000];
			snprintf(log_msg, sizeof(log_msg), 
				"{\"event_type\":\"log\",\"payload\":"
				"{\"service_id\":\"ac_1\", "
				"\"type\":\"access-control\", "
				"\"description\":\"Access granted from online portal.\", "
				"\"event\":\"authentication\", "
				"\"value\":\"true\"}"
				"}");
			addServerMessageToQueue(log_msg);

			for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
				if (!locks[i].enable) continue;
				int ch = i + 1;
				arm_lock(ch, false, true);
			}

			vTaskDelay(locks[0].delay * 1000 / portTICK_PERIOD_MS);

			for (uint8_t i=0; i < NUM_OF_LOCKS; i++) {
				if (!locks[i].enable) continue;
				int ch = i + 1;
				arm_lock(ch, true, true);
			}
		}

		cJSON_Delete(payload);
	}
}

static void lock_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_LOCKS; i++) {
            locks[i].isContact = !get_io(locks[i].contactPin);
        }

        handle_lock_message(checkServiceMessage("lock"));
        handle_lock_message(checkServiceMessageByAction("ac_1", "pulseLock"));

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void lock_main() {
    ESP_LOGI(TAG, "Starting lock service.");
    TaskHandle_t lock_service_task;

    lock_init();
    restoreLockSettings();

    xTaskCreate(&lock_service, "lock_service_task", 8 * 1000, NULL, 5, NULL);
    xTaskCreate(lock_contact_timer, "lock_contact_timer", 2048, NULL, 10, NULL);
}