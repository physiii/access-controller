#include "lock.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "drivers/mcp23x17.h"
#include "automation.h"
#include "gpio.h"
#include "buzzer.h"
#include "store.h"

// Constants
const uint8_t LOCK_SIGNAL_PIN_1 = B1;
const uint8_t LOCK_SIGNAL_PIN_2 = A2;
const uint8_t LOCK_CONTACT_PIN_1 = A1;
const uint8_t LOCK_CONTACT_PIN_2 = B2;

// Global variables
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
Lock locks[NUM_OF_LOCKS];

static const char *LOCK_TAG = "Lock Service";

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
        && !lck->expired) {

        if (!lck->isSignal && !lck->sentSignalAlert) {
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
            ESP_LOGI(LOCK_TAG, "No signal from lock %d, sounding alert.", lck->channel);
        }

        if (!lck->isContact && !lck->sentContactAlert) {
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
            ESP_LOGI(LOCK_TAG, "No contact from lock %d, sounding alert.", lck->channel);
        }
        longBeep(1);
    } else {
        lck->sentSignalAlert = false;
        lck->sentContactAlert = false;
        lck->count++;
    }

    // Check for contact change and send events
    if (lck->isContact != lck->prevIsContact) {
        snprintf(log_msg, sizeof(log_msg),
            "{\"event_type\":\"log\",\"payload\":"
            "{\"service_id\":\"ac_1\", "
            "\"type\":\"access-control\", "
            "\"description\":\"Door %s.\", "
            "\"event\":\"authentication\", "
            "\"value\":\"true\"}"
            "}",
            lck->isContact ? "closed" : "opened");
        addServerMessageToQueue(log_msg);
        ESP_LOGI(LOCK_TAG, "Door %s.", lck->isContact ? "closed" : "opened");
    }

    lck->prevIsContact = lck->isContact;
}

static void lock_contact_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_LOCKS; i++) {
            check_lock_contact_timer(&locks[i]);
        }
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
    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        if (locks[i].channel == ch) {
            locks[i].enable = val;
        }
    }
}

void arm_lock(int channel, bool arm, bool alert) {
    int ch = channel - 1;
    if (!locks[ch].enable) return;
    locks[ch].alert = alert;
    locks[ch].shouldLock = arm;

    if (locks[ch].polarity) {
        arm = !arm;
    }

    set_io(locks[ch].controlPin, arm);
    start_lock_contact_timer(&locks[ch], true);

    if (locks[ch].alert) {
        beep(1);
    }
}

int storeLockSettings() {
    for (uint8_t i = 0; i < NUM_OF_LOCKS; i++) {
        char settings[1000];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"lock\", "
                 "\"payload\":{\"channel\":%d, \"enable\":%s, \"arm\":%s, \"enableContactAlert\":%s, \"polarity\":%s}}",
                 i + 1,
                 locks[i].enable ? "true" : "false",
                 locks[i].shouldLock ? "true" : "false",
                 locks[i].enableContactAlert ? "true" : "false",
                 locks[i].polarity ? "true" : "false");
        
        strncpy(locks[i].settings, settings, sizeof(locks[i].settings));
        snprintf(locks[i].key, sizeof(locks[i].key), "lock%d", i);
        storeSetting(locks[i].key, cJSON_Parse(locks[i].settings));
    }
    return 0;
}

int restoreLockSettings() {
    for (uint8_t i = 0; i < NUM_OF_LOCKS; i++) {
        snprintf(locks[i].key, sizeof(locks[i].key), "lock%d", i);
        restoreSetting(locks[i].key);
        printf("Lock %d key is %s\n", i + 1, locks[i].key);
        vTaskDelay(10 * 000 / portTICK_PERIOD_MS);
    }
    return 0;
}

int sendLockState() {
    for (uint8_t i = 0; i < NUM_OF_LOCKS; i++) {
        char settings[1000];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", "
                 "\"payload\":{\"channel\":%d, \"enable\": %s, \"arm\": %s, \"polarity\": %s, \"enableContactAlert\": %s}}",
                 locks[i].type,
                 i + 1,
                 locks[i].enable ? "true" : "false",
                 locks[i].shouldLock ? "true" : "false",
                 locks[i].polarity ? "true" : "false",
                 locks[i].enableContactAlert ? "true" : "false");

        strncpy(locks[i].settings, settings, sizeof(locks[i].settings));
        addClientMessageToQueue(locks[i].settings);
    }
    return 0;
}

void handle_lock_message(cJSON *payload) {
    if (payload == NULL) return;

    ESP_LOGI(LOCK_TAG, "Handling lock message.");

    int ch = 0;
    bool val = false;
    char property[250];

    cJSON *channel_item = cJSON_GetObjectItem(payload, "channel");
    cJSON *enable_item = cJSON_GetObjectItem(payload, "enable");
    cJSON *enableContactAlert_item = cJSON_GetObjectItem(payload, "enableContactAlert");
    cJSON *polarity_item = cJSON_GetObjectItem(payload, "polarity");
    cJSON *arm_item = cJSON_GetObjectItem(payload, "arm");
    cJSON *property_item = cJSON_GetObjectItem(payload, "property");

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendLockState();
    }

    if (channel_item) {
        ch = channel_item->valueint;

        if (arm_item) {
            val = arm_item->type == cJSON_True;
            arm_lock(ch, val, true);
        }

        if (enable_item) {
            val = cJSON_IsTrue(enable_item);
            enableLock(ch, val);
        }

        if (enableContactAlert_item) {
            val = cJSON_IsTrue(enableContactAlert_item);
            locks[ch - 1].enableContactAlert = val;
        }

        if (polarity_item) {
            ESP_LOGI(LOCK_TAG, "Setting polarity for lock %d", ch);
            locks[ch - 1].polarity = cJSON_IsTrue(polarity_item);
        }

        storeLockSettings();
    }

    if (property_item) {
        if (property_item->valuestring == NULL) {
            return;
        }

        snprintf(property, sizeof(property), "%s", property_item->valuestring);
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

            for (uint8_t i = 0; i < NUM_OF_LOCKS; i++) {
                if (!locks[i].enable) continue;
                int ch = i + 1;
                arm_lock(ch, false, true);
            }

            vTaskDelay(locks[0].delay * 1000 / portTICK_PERIOD_MS);

            for (uint8_t i = 0; i < NUM_OF_LOCKS; i++) {
                if (!locks[i].enable) continue;
                int ch = i + 1;
                arm_lock(ch, true, true);
            }
        }
    }

    // cJSON_Delete(payload);
}

void lock_init() {
    locks[0].channel = 1;
    locks[0].controlPin = LOCK_MCP_IO_1;
    locks[0].isLocked = true;
    locks[0].shouldLock = true;
    locks[0].isSignal = true;
    locks[0].contactPin = LOCK_CONTACT_PIN_1;
    locks[0].signalPin = LOCK_SIGNAL_PIN_1;
    locks[0].enable = true;
    locks[0].delay = 4;
    locks[0].alert = true;
    locks[0].enableContactAlert = false;
    locks[0].polarity = 0;
    strcpy(locks[0].type, "lock");

    locks[1].channel = 2;
    locks[1].controlPin = LOCK_MCP_IO_2;
    locks[1].isLocked = true;
    locks[1].shouldLock = true;
    locks[1].isSignal = true;
    locks[1].contactPin = LOCK_CONTACT_PIN_2;
    locks[1].signalPin = LOCK_SIGNAL_PIN_2;
    locks[1].enable = true;
    locks[1].delay = 4;
    locks[1].alert = true;
    locks[1].enableContactAlert = false;
    locks[1].polarity = 0;
    strcpy(locks[1].type, "lock");

    for (int i = 0; i < NUM_OF_LOCKS; i++) {
        set_io(locks[i].controlPin, false);
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
    ESP_LOGI(LOCK_TAG, "Starting lock service.");
    lock_init();
    restoreLockSettings();
    xTaskCreate(lock_service, "lock_service_task", 8000, NULL, 5, NULL);
    xTaskCreate(lock_contact_timer, "lock_contact_timer", 4000, NULL, 10, NULL);
}
