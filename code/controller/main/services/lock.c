#define LOCK_MCP_IO_1       A0
#define LOCK_MCP_IO_2       B0
#define NUM_OF_LOCKS        2

const uint8_t LOCK_CONTACT_PIN_1 = A1;
const uint8_t LOCK_CONTACT_PIN_2 = B2;

const uint8_t LOCK_SIGNAL_PIN_1 = B1;
const uint8_t LOCK_SIGNAL_PIN_2 = A2;

// const uint8_t LOCK_CONTACT_PIN_1 = B1;
// const uint8_t LOCK_CONTACT_PIN_2 = B1;

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
	uint8_t signalPin;
    uint8_t channel;
    bool isContact;
    bool prevIsContact;
	bool shouldLock;
    bool isLocked;
	bool isSignal;
    bool pulse;
    bool polarity;
    bool expired;
    bool enable;
    bool enableContactAlert;
    int delay;
    int count;
    bool alert;
	bool sentSignalAlert;
	bool sentContactAlert;
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
	if (!lck->enable) return;
	char log_msg[500];

    if (lck->count >= lck->delay
        && lck->shouldLock
        && (!lck->isSignal || !lck->isContact)
        && lck->enableContactAlert
        && lck->enable
        && !lck->expired)
    {
        if (!lck->isSignal) {
            if (!lck->sentSignalAlert) {
                snprintf(log_msg, sizeof(log_msg), 
                    "{\"event_type\":\"log\",\"payload\":"
                    "{\"service_id\":\"ac_1\", "
                    "\"type\":\"access-control\", "
                    "\"description\":\"No armed status coming from lock.\", "
                    "\"event\":\"authentication\", "
                    "\"value\":\"true\"}"
                    "}");
                addServerMessageToQueue(log_msg);
                lck->sentSignalAlert = true;
            }
            ESP_LOGI(TAG, "No signal from lock %d, sounding alert.", lck->channel);
        }
        if (!lck->isContact) {
            if (!lck->sentContactAlert) {
                snprintf(log_msg, sizeof(log_msg), 
                    "{\"event_type\":\"log\",\"payload\":"
                    "{\"service_id\":\"ac_1\", "
                    "\"type\":\"access-control\", "
                    "\"description\":\"Door is still open when armed.\", "
                    "\"event\":\"authentication\", "
                    "\"value\":\"true\"}"
                    "}");
                addServerMessageToQueue(log_msg);
                lck->sentContactAlert = true;
            }
            ESP_LOGI(TAG, "No contact from lock %d, sounding alert.", lck->channel);
        }
        longBeep(1);
    } else {
        lck->sentSignalAlert = false;
        lck->sentContactAlert = false;
        lck->count++;
    }

    // check for contact change and send opened or closed event depending on the contact state
    if (lck->isContact != lck->prevIsContact) {
		if (lck->isContact) {
			snprintf(log_msg, sizeof(log_msg), 
				"{\"event_type\":\"log\",\"payload\":"
				"{\"service_id\":\"ac_1\", "
				"\"type\":\"access-control\", "
				"\"description\":\"Door closed.\", "
				"\"event\":\"authentication\", "
				"\"value\":\"true\"}"
				"}");
			addServerMessageToQueue(log_msg);
			ESP_LOGI(TAG, "Door closed.");
		} else {
			snprintf(log_msg, sizeof(log_msg), 
				"{\"event_type\":\"log\",\"payload\":"
				"{\"service_id\":\"ac_1\", "
				"\"type\":\"access-control\", "
				"\"description\":\"Door opened.\", "
				"\"event\":\"authentication\", "
				"\"value\":\"true\"}"
				"}");
			addServerMessageToQueue(log_msg);
			ESP_LOGI(TAG, "Door opened.");
		}
    }
	lck->prevIsContact = lck->isContact;
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
	locks[ch].shouldLock = arm;

	if (locks[ch].polarity) arm = !arm;

    set_io(locks[ch].controlPin, arm);
    start_lock_contact_timer(&locks[ch], true);

    if (locks[ch].alert) {
        beep(1);
        beep_keypad(1, locks[ch].channel);
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
	cJSON *polarity_item = cJSON_GetObjectItem(payload, "polarity");
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

		if (polarity_item) {
			locks[ch - 1].polarity = polarity_item->type == cJSON_True;
			printf("Polarity for lock %d is %d\n", ch, locks[ch - 1].polarity);
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

void lock_init()
{
    locks[0].channel = 1;
    locks[0].controlPin = USE_MCP23017 ? LOCK_MCP_IO_1 : LOCK_IO_1;
    locks[0].isLocked = true;
	locks[0].shouldLock = true;
	locks[0].isSignal = true;
	locks[0].contactPin = USE_MCP23017 ? LOCK_CONTACT_PIN_1 : CONTACT_IO_1;
	locks[0].signalPin = USE_MCP23017 ? LOCK_SIGNAL_PIN_1 : SIGNAL_IO_1;
	locks[0].enable = true;
	locks[0].delay = 4;
	locks[0].alert = true;
	locks[0].enableContactAlert = false;
	locks[0].polarity = 0;
	strcpy(locks[0].type, "lock");

    locks[1].channel = 2;
    locks[1].controlPin = USE_MCP23017 ? LOCK_MCP_IO_2 : LOCK_IO_2;
    locks[1].isLocked = true;
	locks[1].shouldLock = true;
	locks[1].isSignal = true;
	locks[1].contactPin = USE_MCP23017 ? LOCK_CONTACT_PIN_2 : CONTACT_IO_2;
	locks[1].signalPin = USE_MCP23017 ? LOCK_SIGNAL_PIN_2 : SIGNAL_IO_2;
	locks[1].enable = true;
	locks[1].delay = 4;
	locks[1].alert = true;
	locks[1].enableContactAlert = false;
	locks[0].polarity = 0;
	strcpy(locks[1].type, "lock");

	for (int i=0; i < NUM_OF_LOCKS; i++) {
		if (USE_MCP23017) {
			set_mcp_io_dir(locks[i].controlPin, MCP_OUTPUT);
			set_mcp_io_dir(locks[i].contactPin, MCP_INPUT);
			set_mcp_io_dir(locks[i].signalPin, MCP_INPUT);
		}
	}
}

static void lock_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_LOCKS; i++) {
            locks[i].isContact = !get_io(locks[i].contactPin);
			locks[i].isSignal = get_io(locks[i].signalPin);
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
    xTaskCreate(lock_contact_timer, "lock_contact_timer", 4 * 1000, NULL, 10, NULL);
}