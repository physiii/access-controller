#include "motion.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "automation.h"
#include "lock.h"
#include "gpio.h"
#include "store.h"
#include "drivers/mcp23x17.h"
#include "exit.h"
#include "keypad.h"  // Assuming enableKeypad is declared in this header

// Global variables
char motion_service_message[2000];
bool motion_service_message_ready = false;
cJSON *motion_payload = NULL;
bool MOTION_ALERT = true;
motion_t motions[NUM_OF_MOTIONS];

void start_motion_timer(motion_t *mtn, bool val) {
    if (val) {
        mtn->expired = false;
        mtn->count = 0;
    } else {
        mtn->expired = true;
    }
}

void check_motion_timer(motion_t *mtn) {
    if (mtn->count >= mtn->delay && !mtn->expired) {
        printf("Re-arming lock from motion %d service.\n", mtn->channel);
        arm_lock(mtn->channel, true, mtn->alert);
        mtn->expired = true;
    } else {
        mtn->count++;
    }
}

static void motion_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_MOTIONS; i++) {
            check_motion_timer(&motions[i]);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void check_motions(motion_t *mtn) {
    mtn->isPressed = get_mcp_io(mtn->pin);
    if (!MOMENTARY && mtn->isPressed != mtn->prevPress) {
        arm_lock(mtn->channel, mtn->isPressed, mtn->alert);
        enableExit(mtn->channel, mtn->isPressed);
        enableKeypad(mtn->channel, mtn->isPressed);
    } else if (!mtn->isPressed && mtn->prevPress) {
        arm_lock(mtn->channel, false, mtn->alert);
        start_motion_timer(mtn, true);
    }
    mtn->prevPress = mtn->isPressed;
}

void alertOnMotion(int ch, bool val) {
    for (int i = 0; i < NUM_OF_MOTIONS; i++) {
        if (motions[i].channel == ch) motions[i].alert = val;
    }
}

int storeMotionSettings() {
    for (uint8_t i = 0; i < NUM_OF_MOTIONS; i++) {
        char settings[256];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s}}",
                 motions[i].type, i + 1,
                 motions[i].enable ? "true" : "false",
                 motions[i].alert ? "true" : "false");
        strncpy(motions[i].settings, settings, sizeof(motions[i].settings));
        char key[64];
        snprintf(key, sizeof(key), "%s%d", motions[i].type, i);
        strncpy(motions[i].key, key, sizeof(motions[i].key));
        storeSetting(motions[i].key, cJSON_Parse(motions[i].settings));
    }
    return 0;
}

int restoreMotionSettings() {
    for (uint8_t i = 0; i < NUM_OF_MOTIONS; i++) {
        char temp_key[64]; // Temporary buffer to hold the key
        snprintf(temp_key, sizeof(temp_key), "%s%d", motions[i].type, i);
        strncpy(motions[i].key, temp_key, sizeof(motions[i].key) - 1); // Ensuring we don't overwrite memory
        motions[i].key[sizeof(motions[i].key) - 1] = '\0'; // Null terminate to be safe
        restoreSetting(motions[i].key);
    }
    return 0;
}

int sendMotionState() {
    for (uint8_t i = 0; i < NUM_OF_MOTIONS; i++) {
        char settings[512];  // Temporary buffer for settings
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s}}",
                 motions[i].type, i + 1,
                 motions[i].enable ? "true" : "false",
                 motions[i].alert ? "true" : "false");
        strncpy(motions[i].settings, settings, sizeof(motions[i].settings));
        addClientMessageToQueue(motions[i].settings);
    }
    return 0;
}

void handle_motion_message(cJSON *payload) {
    if (payload == NULL) return;
    int ch = 0;
    bool val = false;

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendMotionState();
    }

    if (cJSON_GetObjectItem(payload, "channel")) {
        ch = cJSON_GetObjectItem(payload, "channel")->valueint;

        if (cJSON_GetObjectItem(payload, "enable")) {
            val = cJSON_IsTrue(cJSON_GetObjectItem(payload, "enable"));
            motions[ch - 1].enable = val;
        }

        if (cJSON_GetObjectItem(payload, "alert")) {
            val = cJSON_IsTrue(cJSON_GetObjectItem(payload, "alert"));
            motions[ch - 1].alert = val;
        }
        storeMotionSettings();
    }
}

static void motion_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_MOTIONS; i++) {
            check_motions(&motions[i]);
        }
        handle_motion_message(checkServiceMessage("motion"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void motion_main() {
    printf("Starting motion service.\n");

    motions[0].pin = MOTION_IO_1;
    motions[0].delay = 4;
    motions[0].channel = 1;
    motions[0].enable = false;
    motions[0].alert = false;
    strcpy(motions[0].type, "motion");

    motions[1].pin = MOTION_IO_2;
    motions[1].delay = 4;
    motions[1].channel = 2;
    motions[1].enable = false;
    motions[1].alert = false;
    strcpy(motions[1].type, "motion");

    if (USE_MCP23017) {
        set_mcp_io_dir(motions[0].pin, MCP_INPUT);
        set_mcp_io_dir(motions[1].pin, MCP_INPUT);
    } else {
        gpio_set_direction(motions[0].pin, GPIO_MODE_INPUT);
        gpio_set_direction(motions[1].pin, GPIO_MODE_INPUT);
    }

    xTaskCreate(motion_timer, "motion_timer", 2048, NULL, 10, NULL);
    xTaskCreate(motion_service, "motion_service", 5000, NULL, 10, NULL);
    restoreMotionSettings();
}
