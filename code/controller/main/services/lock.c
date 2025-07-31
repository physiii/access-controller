#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cJSON.h"
#include "automation.h"

// Function declarations
bool get_io(uint8_t io);
void set_io(uint8_t io, bool val);
void set_mcp_io_dir(uint8_t io, bool dir);
void set_mcp_io(uint8_t io, bool val);
bool get_mcp_io(uint8_t io);
void set_bool(const char *key, bool value);
bool get_bool(const char *key, bool default_value);
void beep(int cnt);
void longBeep(int cnt);

// MCP definitions
#define MCP_OUTPUT 0
#define MCP_INPUT  1

// MCP pin definitions
#define MCP_A0  0
#define MCP_A1  1
#define MCP_A2  2
#define MCP_A3  3
#define MCP_A4  4
#define MCP_A5  5
#define MCP_A6  6
#define MCP_A7  7
#define MCP_B0  8
#define MCP_B1  9
#define MCP_B2  10
#define MCP_B3  11
#define MCP_B4  12
#define MCP_B5  13
#define MCP_B6  14
#define MCP_B7  15

#define LOCK_MCP_IO_1       MCP_A0
#define LOCK_MCP_IO_2       MCP_B0
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
        beep_keypad(1, 50);
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
        beep_keypad(1, 50);
        beep_keypad(1, locks[ch].channel);
    }
}

int storeLockSettings() {
    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        char key[50];
        snprintf(key, sizeof(key), "lock_%d_enable", i + 1);
        set_bool(key, locks[i].enable);
        
        snprintf(key, sizeof(key), "lock_%d_arm", i + 1);
        set_bool(key, locks[i].shouldLock);
        
        snprintf(key, sizeof(key), "lock_%d_contactAlert", i + 1);
        set_bool(key, locks[i].enableContactAlert);
        
        snprintf(key, sizeof(key), "lock_%d_polarity", i + 1);
        set_bool(key, locks[i].polarity);
    }
    return 0;
}

int restoreLockSettings() {
    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        char key[50];
        snprintf(key, sizeof(key), "lock_%d_enable", i + 1);
        locks[i].enable = get_bool(key, true); // Default to enabled
        
        snprintf(key, sizeof(key), "lock_%d_arm", i + 1);
        locks[i].shouldLock = get_bool(key, true); // Default to armed
        
        snprintf(key, sizeof(key), "lock_%d_contactAlert", i + 1);
        locks[i].enableContactAlert = get_bool(key, false); // Default to disabled
        
        snprintf(key, sizeof(key), "lock_%d_polarity", i + 1);
        locks[i].polarity = get_bool(key, false); // Default to normal polarity
    }
    return 0;
}

void sendLockState(void) {
    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "eventType", "lock");
        cJSON *payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(payload, "channel", i + 1);
        cJSON_AddBoolToObject(payload, "enable", locks[i].enable);
        cJSON_AddBoolToObject(payload, "arm", locks[i].shouldLock);
        cJSON_AddBoolToObject(payload, "enableContactAlert", locks[i].enableContactAlert);
        cJSON_AddBoolToObject(payload, "polarity", locks[i].polarity);
        cJSON_AddItemToObject(root, "payload", payload);
        
        addClientMessageToQueue(root);
        cJSON_Delete(root);
    }
}

void handle_lock_message(cJSON * payload) {
    int ch=0;
    bool val;

    if (payload == NULL) return;

	cJSON *channel_item = cJSON_GetObjectItem(payload, "channel");
	cJSON *enable_item = cJSON_GetObjectItem(payload, "enable");
	cJSON *enableContactAlert_item = cJSON_GetObjectItem(payload, "enableContactAlert");
	cJSON *polarity_item = cJSON_GetObjectItem(payload, "polarity");
	cJSON *arm_item = cJSON_GetObjectItem(payload, "arm");
	cJSON *getState_item = cJSON_GetObjectItem(payload,"getState");

	if (getState_item && getState_item->type == cJSON_True) {
		sendLockState();
		cJSON_Delete(payload);
		return;
	}

	if (channel_item) {
	 	ch = channel_item->valueint - 1;
        if (ch < 0 || ch >= NUM_OF_LOCKS) {
            ESP_LOGE(TAG, "Invalid channel: %d", ch + 1);
            cJSON_Delete(payload);
            return;
        }
    }

	if (enable_item) {
		val = (enable_item->type == cJSON_True);
        locks[ch].enable = val;
        ESP_LOGI(TAG, "Enable for lock %d is %d", ch + 1, val);
    }

	if (enableContactAlert_item) {
        val = (enableContactAlert_item->type == cJSON_True);
        locks[ch].enableContactAlert = val;
        ESP_LOGI(TAG, "Enabling contact alert for lock %d", ch + 1);
    }

	if (polarity_item) {
        val = (polarity_item->type == cJSON_True);
        locks[ch].polarity = val;
        ESP_LOGI(TAG, "Polarity for lock %d is %d", ch + 1, val);
    }

	if (arm_item) {
        val = (arm_item->type == cJSON_True);
        locks[ch].shouldLock = val;
        if (val) {
            arm_lock(ch + 1, true, true);
            ESP_LOGI(TAG, "Arming lock %d", ch + 1);
        } else {
            arm_lock(ch + 1, false, true);
            ESP_LOGI(TAG, "Disarming lock %d", ch + 1);
        }
    }

    // Save settings to flash storage
    storeLockSettings();
    
    cJSON_Delete(payload);
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
        handle_lock_message(checkServiceMessage("lock"));
        
        // Reduce frequency of contact checking to prevent spam
        static int contact_check_counter = 0;
        contact_check_counter++;
        
        // Only check contacts every 200 iterations (about 20 seconds)
        if (contact_check_counter >= 200) {
            for (int i = 0; i < NUM_OF_LOCKS; i++) {
                if (locks[i].enableContactAlert && locks[i].enable) {
                    // Check for contact
                    bool hasContact = false;
                    
                    if (i == 0) {
                        hasContact = get_io(CONTACT_IO_1);
                    } else if (i == 1) {
                        hasContact = get_io(CONTACT_IO_2);
                    }
                    
                    if (!hasContact) {
                        ESP_LOGI(TAG, "No contact from lock %d, sounding alert.", i + 1);
                        beep_keypad(1, 200);
                    }
                }
            }
            contact_check_counter = 0;
        }
        
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