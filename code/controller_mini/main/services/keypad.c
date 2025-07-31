#include "keypad.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "automation.h"
#include "lock.h"
#include "store.h"
#include "gpio.h"
#include "drivers/mcp23x17.h"

// Global variables
char keypad_service_message[2000];
bool keypad_service_message_ready = false;
cJSON *keypad_payload = NULL;
keypadButton_t keypads[NUM_OF_KEYPADS];

void start_keypad_timer(keypadButton_t *pad, bool val) {
    if (val) {
        pad->expired = false;
        pad->count = 0;
    } else {
        pad->expired = true;
    }
}

void check_keypad_timer(keypadButton_t *pad) {
    if (pad->count >= pad->delay && !pad->expired) {
        printf("Re-arming lock from pad %d service. Alert %d\n", pad->channel, pad->alert);
        arm_lock(pad->channel, true, pad->alert);
        pad->expired = true;
    } else {
        pad->count++;
    }
}

static void keypad_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_KEYPADS; i++) {
            check_keypad_timer(&keypads[i]);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

int storeKeypadSettings() {
    for (uint8_t i = 0; i < NUM_OF_KEYPADS; i++) {
        char settings[256], key[64];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", "
                 "\"payload\":{\"channel\":%d, \"enable\": \"%s\", \"alert\": \"%s\", \"delay\": %d}}",
                 keypads[i].type, i + 1,
                 keypads[i].enable ? "true" : "false",
                 keypads[i].alert ? "true" : "false",
                 keypads[i].delay);
        snprintf(key, sizeof(key), "%s%d", keypads[i].type, i);
        strncpy(keypads[i].settings, settings, sizeof(keypads[i].settings));
        strncpy(keypads[i].key, key, sizeof(keypads[i].key));
        storeSetting(keypads[i].key, cJSON_Parse(keypads[i].settings));
    }
    return 0;
}

int restoreKeypadSettings() {
    for (uint8_t i = 0; i < NUM_OF_KEYPADS; i++) {
        char temp_key[64];
        snprintf(temp_key, sizeof(temp_key), "%s%d", keypads[i].type, i);
        strncpy(keypads[i].key, temp_key, sizeof(keypads[i].key) - 1);
        keypads[i].key[sizeof(keypads[i].key) - 1] = '\0'; // Ensure null termination
        restoreSetting(keypads[i].key);
    }
    return 0;
}

int sendKeypadState() {
    for (uint8_t i = 0; i < NUM_OF_KEYPADS; i++) {
        char settings[256];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", "
                 "\"payload\":{\"channel\":%d, \"enable\": \"%s\", \"alert\": \"%s\", \"delay\": %d}}",
                 keypads[i].type,
                 i + 1,
                 keypads[i].enable ? "true" : "false",
                 keypads[i].alert ? "true" : "false",
                 keypads[i].delay);
        strncpy(keypads[i].settings, settings, sizeof(keypads[i].settings) - 1);
        keypads[i].settings[sizeof(keypads[i].settings) - 1] = '\0'; // Ensure null termination
        addClientMessageToQueue(keypads[i].settings);
    }
    return 0;
}

void enableKeypad (int ch, bool val) {
    for (int i = 0; i < NUM_OF_KEYPADS; i++) {
        if (keypads[i].channel == ch) keypads[i].enable = val;
    }
}

void alertOnKeypad (int ch, bool val) {
    printf("alertOnKeypad\tch: %d alert: %d.\n", ch, val);
    for (int i = 0; i < NUM_OF_KEYPADS; i++) {
        if (keypads[i].channel == ch) keypads[i].alert = val;
    }
}

void setKeypadArmDelay (int ch, int val) {
    for (int i = 0; i < NUM_OF_KEYPADS; i++) {
        if (keypads[i].channel == ch) keypads[i].delay = val;
    }
}

void check_keypads (keypadButton_t *pad) {
    if (!pad->enable) return;

    pad->isPressed = !get_io(pad->pin);

    if (pad->isPressed && !pad->prevPress) {
        printf("Disarming lock from pad %d service. Alert %d\n", pad->channel, pad->alert);
        // beep_keypad(1, pad->channel);
        arm_lock(pad->channel, false, pad->alert);
        start_keypad_timer(pad, true);
    }

    pad->prevPress = pad->isPressed;
}

void handle_keypad_message(cJSON *payload) {
    int ch = 0;
    bool tmp;
    if (payload == NULL) return;

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendKeypadState();
    }

    if (cJSON_GetObjectItem(payload, "channel")) {
        ch = cJSON_GetObjectItem(payload, "channel")->valueint;
        if (cJSON_GetObjectItem(payload, "alert")) {
            tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload, "alert"));
            alertOnKeypad(ch, tmp);
        }
        if (cJSON_GetObjectItem(payload, "enable")) {
            tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload, "enable"));
            enableKeypad(ch, tmp);
        }
        if (cJSON_GetObjectItem(payload, "delay")) {
            setKeypadArmDelay(ch, cJSON_GetObjectItem(payload, "delay")->valueint);
        }
        storeKeypadSettings();
    }
}

static void keypad_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_KEYPADS; i++) {
            check_keypads(&keypads[i]);
        }
        handle_keypad_message(checkServiceMessage("keypad"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void keypad_main() {
    printf("Starting keypad service.\n");
    keypads[0].pin = USE_MCP23017 ? KEYPAD_MCP_IO_1 : KEYPAD_IO_1;
    keypads[0].delay = 4;
    keypads[0].channel = 1;
    keypads[0].alert = true;
    keypads[0].enable = true;
    strcpy(keypads[0].type, "keypad");

    keypads[1].pin = USE_MCP23017 ? KEYPAD_MCP_IO_2 : KEYPAD_IO_2;
    keypads[1].delay = 4;
    keypads[1].channel = 2;
    keypads[1].enable = true;
    keypads[1].alert = true;
    strcpy(keypads[1].type, "keypad");

    if (USE_MCP23017) {
        set_mcp_io_dir(keypads[0].pin, MCP_INPUT);
        set_mcp_io_dir(keypads[1].pin, MCP_INPUT);
    } else {
        gpio_set_direction(keypads[0].pin, GPIO_MODE_INPUT);
        gpio_set_direction(keypads[1].pin, GPIO_MODE_INPUT);
    }

    xTaskCreate(keypad_timer, "keypad_timer", 2048, NULL, 10, NULL);
    xTaskCreate(keypad_service, "keypad_service", 2048, NULL, 10, NULL);
    restoreKeypadSettings();
}
