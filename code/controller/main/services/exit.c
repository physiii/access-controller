#include "exit.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "automation.h"
#include "lock.h"
#include "store.h"
#include "gpio.h"
#include "buzzer.h"
#include "drivers/mcp23x17.h"

// Global variables
const char *EXIT_TAG = "exit";
char exit_service_message[2000];
bool exit_service_message_ready = false;
cJSON *exit_payload = NULL;
exitButton_t exits[NUM_OF_EXITS];

void start_exit_timer(exitButton_t *ext, bool val) {
    if (val) {
        ext->expired = false;
        ext->count = 0;
    } else {
        ext->expired = true;
    }
}

void check_exit_timer(exitButton_t *ext) {
    if (ext->count >= ext->delay && !ext->expired) {
        printf("Re-arming lock from button %d service.\n", ext->channel);
        arm_lock(ext->channel, true, ext->alert);
        ext->expired = true;
    } else {
        ext->count++;
    }
}

static void exit_timer(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_EXITS; i++) {
            check_exit_timer(&exits[i]);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void storeExitSettings() {
    for (uint8_t i = 0; i < NUM_OF_EXITS; i++) {
        char settings[1000];
        char temp_key[64];
        snprintf(settings, sizeof(settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
                 exits[i].type, i + 1,
                 exits[i].enable ? "true" : "false",
                 exits[i].alert ? "true" : "false",
                 exits[i].delay);
        snprintf(temp_key, sizeof(temp_key), "%s%d", exits[i].type, i);
        strncpy(exits[i].settings, settings, sizeof(exits[i].settings) - 1);
        strncpy(exits[i].key, temp_key, sizeof(exits[i].key) - 1);
        storeSetting(exits[i].key, cJSON_Parse(exits[i].settings));
    }
}

void restoreExitSettings() {
    for (uint8_t i = 0; i < NUM_OF_EXITS; i++) {
        char temp_key[64];
        snprintf(temp_key, sizeof(temp_key), "%s%d", exits[i].type, i);
        strncpy(exits[i].key, temp_key, sizeof(exits[i].key) - 1);
        restoreSetting(exits[i].key);
    }
}

int sendExitState() {
    for (uint8_t i = 0; i < NUM_OF_EXITS; i++) {
        char temp_settings[256];
        snprintf(temp_settings, sizeof(temp_settings),
                 "{\"eventType\":\"%s\", \"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
                 exits[i].type, i + 1,
                 exits[i].enable ? "true" : "false",
                 exits[i].alert ? "true" : "false",
                 exits[i].delay);
        strncpy(exits[i].settings, temp_settings, sizeof(exits[i].settings) - 1);
        exits[i].settings[sizeof(exits[i].settings) - 1] = '\0'; // Ensure null termination
        addClientMessageToQueue(exits[i].settings);
    }
    return 0;
}

void check_exit(exitButton_t *ext) {
    if (!ext->enable) return;
    ext->isPressed = !get_io(ext->pin);
    if (ext->isPressed && !ext->prevPress) {
        arm_lock(ext->channel, false, ext->alert);
        start_exit_timer(ext, true);
    }
    ext->prevPress = ext->isPressed;
}

void enableExit (int ch, bool val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].enable = val;
}

void alertOnExit (int ch, bool val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].alert = val;
}

void setArmDelay (int ch, int val)
{
	for (int i=0; i < NUM_OF_EXITS; i++)
		if (exits[i].channel == ch) exits[i].delay = val;
}

void handle_exit_message(cJSON *payload) {
    int ch = 0;
    bool tmp = false;

    if (payload == NULL) return;

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendExitState();
    }

    if (cJSON_GetObjectItem(payload, "channel")) {
        ch = cJSON_GetObjectItem(payload, "channel")->valueint;

        if (cJSON_GetObjectItem(payload, "alert")) {
            tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload, "alert"));
            alertOnExit(ch, tmp);
        }

        if (cJSON_GetObjectItem(payload, "enable")) {
            tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload, "enable"));
            enableExit(ch, tmp);
        }

        if (cJSON_GetObjectItem(payload, "delay")) {
            setArmDelay(ch, cJSON_GetObjectItem(payload, "delay")->valueint);
        }

        ESP_LOGI(EXIT_TAG, "Exit message: %s", cJSON_PrintUnformatted(payload));

        storeExitSettings();
    }
}

static void exit_service(void *pvParameter) {
    while (1) {
        for (int i = 0; i < NUM_OF_EXITS; i++) {
            check_exit(&exits[i]);
        }
        handle_exit_message(checkServiceMessage("exit"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void exit_main() {
    printf("Starting exit service.\n");
    exits[0].pin = USE_MCP23017 ? EXIT_BUTTON_MCP_IO_1 : EXIT_BUTTON_IO_1;
    exits[0].delay = 4;
    exits[0].channel = 1;
    exits[0].alert = false;
    exits[0].enable = false;
    strcpy(exits[0].type, "exit");

    exits[1].pin = USE_MCP23017 ? EXIT_BUTTON_MCP_IO_2 : EXIT_BUTTON_IO_2;
    exits[1].delay = 4;
    exits[1].channel = 2;
    exits[1].alert = false;
    exits[1].enable = false;
    strcpy(exits[1].type, "exit");

    restoreExitSettings();

    if (USE_MCP23017) {
        set_mcp_io_dir(exits[0].pin, MCP_INPUT);
        set_mcp_io_dir(exits[1].pin, MCP_INPUT);
    } else {
        gpio_set_direction(exits[0].pin, GPIO_MODE_INPUT);
        gpio_set_direction(exits[1].pin, GPIO_MODE_INPUT);
    }

    xTaskCreate(exit_timer, "exit_timer", 2048, NULL, 10, NULL);
    xTaskCreate(exit_service, "exit_service", 5000, NULL, 10, NULL);
}
