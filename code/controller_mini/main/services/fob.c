#include "fob.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "automation.h"
#include "lock.h"
#include "drivers/mcp23x17.h"
#include "store.h"

// Global variables
char fob_service_message[2000];
bool fob_service_message_ready = false;
cJSON *fob_payload = NULL;
fob_t fobs[NUM_OF_FOBS];

void start_fob_timer(fob_t *fb, bool val) {
    if (val) {
        fb->expired = false;
        fb->count = 0;
    } else {
        fb->expired = true;
    }
}

void check_fob_timer(fob_t *fb) {
    if (fb->count >= fb->delay && !fb->expired) {
        printf("Re-arming lock from fob %d service.\n", fb->channel);
        arm_lock(fb->channel, true, fb->alert);
        fb->expired = true;
    } else {
        fb->count++;
    }
}

static void fob_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_FOBS; i++) {
            check_fob_timer(&fobs[i]);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void check_fobs(fob_t *fb) {
    if (!fb->enable) return;
    fb->isPressed = get_mcp_io(fb->pin);
    
    if (fb->latch && fb->isPressed != fb->prevPress) {
        // Latch mode: FOB state directly controls lock state
        if (fb->isPressed) {
            ESP_LOGI("FOB", "Fob %d activated (latch mode) on channel %d", fb->pin, fb->channel);
        }
        arm_lock(fb->channel, fb->isPressed, fb->alert);
    } else if (!fb->latch && !fb->isPressed && fb->prevPress) {
        // Momentary mode: FOB release triggers unlock and timer
        ESP_LOGI("FOB", "Fob %d released (momentary mode) - disarming lock on channel %d", fb->pin, fb->channel);
        arm_lock(fb->channel, false, fb->alert);
        start_fob_timer(fb, true);
    }
    fb->prevPress = fb->isPressed;
}

void storeFobSettings() {
    for (uint8_t i = 0; i < NUM_OF_FOBS; i++) {
        char temp_settings[256];
        snprintf(temp_settings, sizeof(temp_settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"latch\": %s}}",
                 fobs[i].type, i + 1,
                 fobs[i].enable ? "true" : "false",
                 fobs[i].alert ? "true" : "false",
                 fobs[i].latch ? "true" : "false");
        strncpy(fobs[i].settings, temp_settings, sizeof(fobs[i].settings) - 1);
        storeSetting(fobs[i].key, cJSON_Parse(fobs[i].settings));
    }
}

void restoreFobSettings() {
    for (uint8_t i = 0; i < NUM_OF_FOBS; i++) {
        char temp_key[64];
        snprintf(temp_key, sizeof(temp_key), "%s%d", fobs[i].type, i);
        strncpy(fobs[i].key, temp_key, sizeof(fobs[i].key) - 1);
        fobs[i].key[sizeof(fobs[i].key) - 1] = '\0';
        restoreSetting(fobs[i].key);
    }
}

int sendFobState() {
    for (uint8_t i = 0; i < NUM_OF_FOBS; i++) {
        char temp_settings[256];
        snprintf(temp_settings, sizeof(temp_settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"latch\": %s}}",
                 fobs[i].type, i + 1,
                 fobs[i].enable ? "true" : "false",
                 fobs[i].alert ? "true" : "false",
                 fobs[i].latch ? "true" : "false");
        strncpy(fobs[i].settings, temp_settings, sizeof(fobs[i].settings) - 1);
        fobs[i].settings[sizeof(fobs[i].settings) - 1] = '\0';
        addClientMessageToQueue(fobs[i].settings);
    }
    return 0;
}

void handle_fob_message(cJSON *payload) {
    int ch = 0;
    bool val = false;

    if (payload == NULL) return;

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendFobState();
    }

    if (cJSON_GetObjectItem(payload, "channel")) {
        ch = cJSON_GetObjectItem(payload, "channel")->valueint;

        if (cJSON_GetObjectItem(payload, "enable")) {
            val = cJSON_IsTrue(cJSON_GetObjectItem(payload, "enable"));
            fobs[ch - 1].enable = val;
        }

        if (cJSON_GetObjectItem(payload, "alert")) {
            val = cJSON_IsTrue(cJSON_GetObjectItem(payload, "alert"));
            fobs[ch - 1].alert = val;
        }

        if (cJSON_GetObjectItem(payload, "latch")) {
            val = cJSON_IsTrue(cJSON_GetObjectItem(payload, "latch"));
            fobs[ch - 1].latch = val;
        }
        
        storeFobSettings();
    }
}

static void fob_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_FOBS; i++) {
            check_fobs(&fobs[i]);
        }
        handle_fob_message(checkServiceMessage("fob"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void fob_main() {
    printf("Starting fob service.\n");
    fobs[0].pin = FOB_IO_1;
    fobs[0].delay = 4;
    fobs[0].channel = 1;
    fobs[0].enable = true;
    fobs[0].alert = true;
    fobs[0].latch = false;  // Default to momentary mode
    strcpy(fobs[0].type, "fob");

    fobs[1].pin = FOB_IO_2;
    fobs[1].delay = 4;
    fobs[1].channel = 2;
    fobs[1].enable = true;
    fobs[1].alert = true;
    fobs[1].latch = false;  // Default to momentary mode
    strcpy(fobs[1].type, "fob");

    if (USE_MCP23017) {
        set_mcp_io_dir(fobs[0].pin, MCP_INPUT);
        set_mcp_io_dir(fobs[1].pin, MCP_INPUT);
    } else {
        gpio_set_direction(fobs[0].pin, GPIO_MODE_INPUT);
        gpio_set_direction(fobs[1].pin, GPIO_MODE_INPUT);
    }

    xTaskCreate(fob_timer, "fob_timer", 2048, NULL, 10, NULL);
    xTaskCreate(fob_service, "fob_service", 5000, NULL, 10, NULL);
    restoreFobSettings();
}
