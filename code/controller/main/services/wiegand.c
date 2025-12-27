#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "wiegand_registry.h"
#include "wiegand.h"
#include "automation.h"

/* Forward declarations */
struct wiegand;
void start_wiegand_timer(struct wiegand *ctx, bool val);
static bool handleKeyCode(struct wiegand *ctx);
int is_pin_authorized(const char *incomingPin);
void arm_lock(int channel, bool arm, bool alert);
void beep_keypad(int beeps, int channel);

#define WIEGAND_FRAME_TIMEOUT_MS 80
#define WIEGAND_MIN_FRAME_BITS   24
#define NUM_OF_WIEGANDS         2
#define NUM_OF_KEYS             12
#define KEYCODE_LENGTH          6
#define WIEGAND_SESSION_MAX     64

typedef struct {
    uint32_t gpio_num;
    int wg_index;
    int gpio_val;
} gpio_event_t;

struct wiegand {
	uint8_t pin0;
	uint8_t pin1;
	uint8_t pin_push;
	char code[25];
	uint8_t bitCount;
	char name[100];
	int count;
	int keypressCount;
	bool expired;
	bool keypressExpired;
	bool enable;
	bool newKey;
	bool newCode;
	char incomingCode[50];
	char fingerCode[50];
	uint8_t incomingCodeCount;
	int keyCount;
	bool alert;
	int delay;
	int keypressTimeout;
	int channel;
	char bit_buffer[WIEGAND_USER_CODE_MAX];
	size_t bit_buffer_len;
	int64_t last_bit_time_us;
};

typedef struct {
    bool active;
    uint8_t channel;
    size_t added_count;
    char added_ids[WIEGAND_SESSION_MAX][WIEGAND_USER_ID_MAX];
    char last_duplicate[WIEGAND_USER_CODE_MAX];
} wiegand_registration_session_t;

static const char *LOG_TAG_WIEGAND = "wiegand";
static struct wiegand wg[NUM_OF_WIEGANDS];
static QueueHandle_t gpio_evt_queue = NULL;
static portMUX_TYPE registrationMutex = portMUX_INITIALIZER_UNLOCKED;
static wiegand_registration_session_t registration_session = {0};

static void wiegand_bits_to_hex(const char *bit_string, size_t bit_len, char *hex_buf, size_t hex_buf_size) {
    static const char HEX[] = "0123456789ABCDEF";
    size_t out = 0;
    uint8_t acc = 0;
    int acc_bits = 0;

    if (!bit_string || bit_len == 0 || !hex_buf || hex_buf_size < 2) {
        if (hex_buf && hex_buf_size > 0) hex_buf[0] = '\0';
        return;
    }

    for (size_t i = 0; i < bit_len && out < hex_buf_size - 1; i++) {
        acc = (acc << 1) | (bit_string[i] == '1');
        acc_bits++;
        if (acc_bits == 4) {
            hex_buf[out++] = HEX[acc & 0x0F];
            acc = 0;
            acc_bits = 0;
        }
    }
    if (acc_bits > 0 && out < hex_buf_size - 1) {
        acc <<= (4 - acc_bits);
        hex_buf[out++] = HEX[acc & 0x0F];
    }
    hex_buf[out] = '\0';
}

static void wiegand_log_frame_hex(const char *bit_string, size_t bit_len, int channel, bool short_frame) {
    if (!bit_string || bit_len == 0) {
        return;
    }
    (void)short_frame;  // No longer used for logging

    char hex_buf[WIEGAND_USER_CODE_MAX / 4 + 4];
    wiegand_bits_to_hex(bit_string, bit_len, hex_buf, sizeof(hex_buf));

    ESP_LOGI(LOG_TAG_WIEGAND, "Ch%d: 0x%s (%u bits)", channel, hex_buf, (unsigned)bit_len);
}

static int IRAM_ATTR wiegand_index_for_gpio(uint32_t gpio_num) {
    for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
        if (wg[i].pin0 == gpio_num || wg[i].pin1 == gpio_num) {
            return i;
        }
    }
    return -1;
}

static bool registration_channel_matches(uint8_t configured_channel, int channel) {
    return configured_channel == 0 || configured_channel == (uint8_t)channel;
}

static void registration_clear_session_locked(void) {
    registration_session.active = false;
    registration_session.channel = 0;
    registration_session.added_count = 0;
    registration_session.last_duplicate[0] = '\0';
    memset(registration_session.added_ids, 0, sizeof(registration_session.added_ids));
}

static void registration_track_new_user(const wiegand_user_t *user) {
    if (!user) {
        return;
    }
    portENTER_CRITICAL(&registrationMutex);
    if (registration_session.added_count < WIEGAND_SESSION_MAX) {
        size_t index = registration_session.added_count;
        snprintf(registration_session.added_ids[index], WIEGAND_USER_ID_MAX, "%s", user->id);
        registration_session.added_count++;
    }
    registration_session.last_duplicate[0] = '\0';
    portEXIT_CRITICAL(&registrationMutex);
}

static void registration_note_duplicate(const char *code) {
    portENTER_CRITICAL(&registrationMutex);
    if (code && code[0] != '\0') {
        snprintf(registration_session.last_duplicate, sizeof(registration_session.last_duplicate), "%s", code);
    } else {
        registration_session.last_duplicate[0] = '\0';
    }
    portEXIT_CRITICAL(&registrationMutex);
}

static bool registration_snapshot(uint8_t *channel_out, size_t *pending_out) {
    bool active;
    portENTER_CRITICAL(&registrationMutex);
    active = registration_session.active;
    if (channel_out) {
        *channel_out = registration_session.channel;
    }
    if (pending_out) {
        *pending_out = registration_session.added_count;
    }
    portEXIT_CRITICAL(&registrationMutex);
    return active;
}

static size_t registration_collect_ids(char ids[][WIEGAND_USER_ID_MAX], size_t max_ids, uint8_t *channel_out, bool *was_active_out) {
    size_t count = 0;
    bool was_active = false;
    portENTER_CRITICAL(&registrationMutex);
    was_active = registration_session.active;
    if (channel_out) {
        *channel_out = registration_session.channel;
    }
    if (was_active && registration_session.added_count > 0) {
        count = registration_session.added_count;
        if (count > max_ids) {
            count = max_ids;
        }
        for (size_t i = 0; i < count; i++) {
            snprintf(ids[i], WIEGAND_USER_ID_MAX, "%s", registration_session.added_ids[i]);
        }
    }
    registration_clear_session_locked();
    portEXIT_CRITICAL(&registrationMutex);
    if (was_active_out) {
        *was_active_out = was_active;
    }
    return was_active ? count : 0;
}

static size_t registration_pending_count(void) {
    size_t count;
    portENTER_CRITICAL(&registrationMutex);
    count = registration_session.added_count;
    portEXIT_CRITICAL(&registrationMutex);
    return count;
}

static void wiegand_reset_bit_buffer(struct wiegand *ctx) {
    if (!ctx) {
        return;
    }
    ctx->bit_buffer_len = 0;
    ctx->bit_buffer[0] = '\0';
    ctx->last_bit_time_us = 0;
}

static void wiegand_process_code(struct wiegand *wg_entry, const char *bit_string) {
    if (!wg_entry || !bit_string || bit_string[0] == '\0') {
        return;
    }

    size_t bit_len = strlen(bit_string);
    (void)bit_len;  // Used below in registration

    uint8_t configured_channel = 0;
    size_t pending = 0;
    bool active = registration_snapshot(&configured_channel, &pending);
    if (active && registration_channel_matches(configured_channel, wg_entry->channel)) {
        wiegand_user_t new_user;
        esp_err_t err = wiegand_registry_add(bit_string, wg_entry->channel, &new_user);
        if (err == ESP_OK) {
            if (new_user.status != WIEGAND_USER_STATUS_PENDING) {
                esp_err_t status_err = wiegand_registry_update_status(new_user.id, WIEGAND_USER_STATUS_PENDING);
                if (status_err == ESP_OK) {
                    new_user.status = WIEGAND_USER_STATUS_PENDING;
                } else {
                    ESP_LOGW(LOG_TAG_WIEGAND,
                             "Failed to mark Wiegand user %s as pending (%s)",
                             new_user.id,
                             esp_err_to_name(status_err));
                }
            }
            registration_track_new_user(&new_user);
            ESP_LOGI(LOG_TAG_WIEGAND,
                     "Captured Wiegand code %s on channel %d (user_id=%s, pending=%u)",
                     bit_string,
                     wg_entry->channel,
                     new_user.id,
                     (unsigned)(pending + 1));
        } else if (err == ESP_ERR_INVALID_STATE) {
            registration_note_duplicate(bit_string);
            ESP_LOGW(LOG_TAG_WIEGAND, "Duplicate Wiegand code %s on channel %d", bit_string, wg_entry->channel);
        } else {
            ESP_LOGE(LOG_TAG_WIEGAND, "Failed to record Wiegand code %s (%s)", bit_string, esp_err_to_name(err));
        }
        return;
    }
    if (active) {
        ESP_LOGD(LOG_TAG_WIEGAND,
                 "Registration active for channel %u but ignoring frame on channel %d",
                 (unsigned)configured_channel,
                 wg_entry->channel);
    }

    const wiegand_user_t *user = wiegand_registry_find_by_code(bit_string);
    if (user && user->status == WIEGAND_USER_STATUS_ACTIVE) {
        const char *display_name = (user->name[0] != '\0') ? user->name : "Wiegand User";
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg),
                 "{\"event_type\":\"log\",\"payload\":{\"service_id\":\"ac_1\",\"type\":\"access-control\",\"description\":\"%s authorized via Wiegand\",\"event\":\"authentication\",\"value\":\"%s\"}}",
                 display_name, bit_string);
        // addServerMessageToQueue(log_msg); // This line is removed
        ESP_LOGI(LOG_TAG_WIEGAND, "Authorized Wiegand code %s (user=%s)", bit_string, display_name);
        lock_set_action_source("wg_code");
        arm_lock(wg_entry->channel, false, wg_entry->alert);
        start_wiegand_timer(wg_entry, true);
    } else {
        ESP_LOGW(LOG_TAG_WIEGAND, "Unauthorized Wiegand code %s on channel %d", bit_string, wg_entry->channel);
        beep_keypad(2, wg_entry->channel);
    }
}

static void IRAM_ATTR wiegand_isr_handler(void *arg) {
	gpio_event_t event;
    event.gpio_num = (uint32_t)arg;
	event.gpio_val = gpio_get_level(event.gpio_num);
    event.wg_index = wiegand_index_for_gpio(event.gpio_num);
	
	// Log all ISR events (note: ESP_LOG* cannot be used in ISR, so we rely on task logging)
	xQueueSendFromISR(gpio_evt_queue, &event, NULL);
}

void start_keypress_timer(struct wiegand *ctx, bool val) {
	if (val) {
        ctx->keypressExpired = false;
        ctx->keypressCount = 0;
	} else {
        ctx->keypressExpired = true;
	}
}

void check_keypress_timer(struct wiegand *ctx) {
    if (ctx->keypressCount >= ctx->keypressTimeout && !ctx->keypressExpired) {
        ESP_LOGW(LOG_TAG_WIEGAND, "Keypress timer expired for wg %d", ctx->channel);
        memset(ctx->code, 0, sizeof(ctx->code));
        memset(ctx->incomingCode, 0, sizeof(ctx->incomingCode));
        ctx->incomingCodeCount = 0;
        ctx->keypressExpired = true;
        ctx->keypressCount = 0;
        beep_keypad(2, ctx->channel);
    } else {
        ctx->keypressCount++;
    }
}

static void keypress_timer(void *pvParameter) {
  while (1) {
        for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
		check_keypress_timer(&wg[i]);
        }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void start_wiegand_timer(struct wiegand *ctx, bool val) {
    start_keypress_timer(ctx, false);
  if (val) {
        ctx->expired = false;
        ctx->count = 0;
  } else {
        ctx->expired = true;
        ctx->incomingCodeCount = 0;
  }
}

void check_wiegand_timer(struct wiegand *ctx) {
    if (!ctx->enable) {
        return;
    }
    if (ctx->count >= ctx->delay && !ctx->expired) {
        ESP_LOGI(LOG_TAG_WIEGAND, "Re-arming lock from wg %d service. Alert %d", ctx->channel, ctx->alert);
        lock_set_action_source("wg_auto");
        arm_lock(ctx->channel, true, ctx->alert);
        ctx->expired = true;
    } else {
        ctx->count++;
    }
}

static void wiegand_timer(void *pvParameter) {
  while (1) {
        for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
			check_wiegand_timer(&wg[i]);
        }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static bool handleKeyCode(struct wiegand *ctx) {
    static const uint8_t key[NUM_OF_KEYS] = {
        0b00000000, 0b11000000, 0b00110000, 0b11110000,
        0b00001100, 0b11001100, 0b00111100, 0b11111100,
        0b00000011, 0b11000011, 0b00110011, 0b11110011
    };

    if (!ctx) {
        return false;
    }

    uint8_t incomingByte = (uint8_t)strtol(ctx->incomingCode, NULL, 2);
    int keyIndex = -1;
    start_keypress_timer(ctx, true);

    for (int i = 0; i < NUM_OF_KEYS; i++) {
        if (incomingByte == key[i]) {
            keyIndex = i;
            break;
        }
    }

    if (keyIndex >= 0 && keyIndex <= 9) {
        size_t len = strlen(ctx->code);
        if (len + 1 < sizeof(ctx->code)) {
            ctx->code[len] = (char)('0' + keyIndex);
            ctx->code[len + 1] = '\0';
        }
        ESP_LOGI(LOG_TAG_WIEGAND, "Keypad digit '%d' received on channel %d (PIN=%s)", keyIndex, ctx->channel, ctx->code);
    } else if (keyIndex == 10) {
        size_t len = strlen(ctx->code);
        if (len + 1 < sizeof(ctx->code)) {
            ctx->code[len] = '*';
            ctx->code[len + 1] = '\0';
        }
        ESP_LOGI(LOG_TAG_WIEGAND, "Keypad '*' received on channel %d", ctx->channel);
    } else if (keyIndex == 11) {
        // Only attempt authorization if PIN is not empty
        if (strlen(ctx->code) > 0) {
            if (is_pin_authorized(ctx->code)) {
                lock_set_action_source("wg_pin");
                arm_lock(ctx->channel, false, ctx->alert);
                start_wiegand_timer(ctx, true);
                ESP_LOGI(LOG_TAG_WIEGAND, "PIN accepted on channel %d (%s)", ctx->channel, ctx->code);
            } else {
                beep_keypad(2, ctx->channel);
                ESP_LOGW(LOG_TAG_WIEGAND, "PIN rejected on channel %d (%s)", ctx->channel, ctx->code);
            }
        } else {
            ESP_LOGD(LOG_TAG_WIEGAND, "Ignoring # key with empty PIN on channel %d", ctx->channel);
        }
        memset(ctx->code, 0, sizeof(ctx->code));
        start_keypress_timer(ctx, false);
        return true;
    }

    if (strlen(ctx->code) > KEYCODE_LENGTH) {
        ESP_LOGW(LOG_TAG_WIEGAND, "Exceeded max keycode length (%d): %s", KEYCODE_LENGTH, ctx->code);
        memset(ctx->code, 0, sizeof(ctx->code));
        memset(ctx->incomingCode, 0, sizeof(ctx->incomingCode));
        ctx->incomingCodeCount = 0;
        start_keypress_timer(ctx, false);
        beep_keypad(2, ctx->channel);
	}

    if (keyIndex == -1) {
        ESP_LOGD(LOG_TAG_WIEGAND, "No keypad match for pattern %s on channel %d", ctx->incomingCode, ctx->channel);
    }

    memset(ctx->incomingCode, 0, sizeof(ctx->incomingCode));
    ctx->incomingCodeCount = 0;
    return keyIndex != -1;
}

static void wiegand_task(void *pvParameter) {
    gpio_event_t event;
    const TickType_t wait_ticks = pdMS_TO_TICKS(WIEGAND_FRAME_TIMEOUT_MS);

    for (;;) {
        bool received = xQueueReceive(gpio_evt_queue, &event, wait_ticks) == pdTRUE;
        int64_t now_us = esp_timer_get_time();

        if (received) {
            ESP_LOGD(LOG_TAG_WIEGAND, 
                     "RAW GPIO event: GPIO=%u, level=%d, wg_index=%d",
                     (unsigned)event.gpio_num, event.gpio_val, event.wg_index);
            
            if (event.wg_index < 0 || event.wg_index >= NUM_OF_WIEGANDS) {
                ESP_LOGD(LOG_TAG_WIEGAND, "Unknown Wiegand index (%d) on GPIO %u", 
                         event.wg_index, (unsigned)event.gpio_num);
                continue;
            }

            struct wiegand *current_wg = &wg[event.wg_index];
            
            if (!current_wg->enable) {
                ESP_LOGD(LOG_TAG_WIEGAND, "Ignoring GPIO %u event for disabled channel %d",
                         (unsigned)event.gpio_num, current_wg->channel);
                continue;
            }

            if (event.gpio_val != 0) {
                continue;  // Ignore rising edge
            }

            char bit = '\0';
            if (event.gpio_num == current_wg->pin0) {
                bit = '0';
            } else if (event.gpio_num == current_wg->pin1) {
                bit = '1';
            } else {
                continue;
            }

            if (current_wg->bit_buffer_len + 1 >= sizeof(current_wg->bit_buffer)) {
                ESP_LOGW(LOG_TAG_WIEGAND, "Frame overflow on channel %d", current_wg->channel);
                wiegand_reset_bit_buffer(current_wg);
                current_wg->incomingCodeCount = 0;
                memset(current_wg->incomingCode, 0, sizeof(current_wg->incomingCode));
                continue;
            }

            current_wg->bit_buffer[current_wg->bit_buffer_len++] = bit;
            current_wg->bit_buffer[current_wg->bit_buffer_len] = '\0';
            current_wg->last_bit_time_us = now_us;

            // Log first bit to indicate frame start
            if (current_wg->bit_buffer_len == 1) {
                ESP_LOGI(LOG_TAG_WIEGAND, "Ch%d: Frame started (bit=%c)", 
                         current_wg->channel, bit);
            }

            current_wg->incomingCodeCount++;
            // Note: We no longer call handleKeyCode mid-frame. 
            // All frame processing happens after timeout to prevent
            // card data being discarded when first 8 bits match keypad patterns.
        }

        for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
            struct wiegand *ctx = &wg[i];
            if (!ctx->enable || ctx->bit_buffer_len == 0) {
                continue;
            }

            if (ctx->last_bit_time_us == 0) {
                ctx->last_bit_time_us = now_us;
                continue;
            }

            if ((now_us - ctx->last_bit_time_us) > (int64_t)WIEGAND_FRAME_TIMEOUT_MS * 1000) {
                ESP_LOGI(LOG_TAG_WIEGAND, "Ch%d: Frame complete - %u bits received", 
                         ctx->channel, (unsigned)ctx->bit_buffer_len);
                
                if (ctx->bit_buffer_len < WIEGAND_MIN_FRAME_BITS) {
                    // Short frames (< 24 bits) are keypad presses
                    // Process 8-bit keypad codes
                    if (ctx->bit_buffer_len >= 4 && ctx->bit_buffer_len <= 8) {
                        // Prepare 8-bit pattern from the bit buffer
                        char keypad_bits[9] = {'0','0','0','0','0','0','0','0','\0'};
                        size_t start = (ctx->bit_buffer_len <= 8) ? (8 - ctx->bit_buffer_len) : 0;
                        for (size_t j = 0; j < ctx->bit_buffer_len && j < 8; j++) {
                            keypad_bits[start + j] = ctx->bit_buffer[j];
                        }
                        memcpy(ctx->incomingCode, keypad_bits, 9);
                        ctx->incomingCodeCount = 8;
                        handleKeyCode(ctx);
                        ctx->incomingCodeCount = 0;
                        memset(ctx->incomingCode, 0, sizeof(ctx->incomingCode));
                    } else {
                        ESP_LOGD(LOG_TAG_WIEGAND, "Ch%d: Ignoring %u-bit short frame (not keypad)", 
                                 ctx->channel, (unsigned)ctx->bit_buffer_len);
                    }
                } else {
                    // Long frames (>= 24 bits) are RFID cards
                    char captured_code[WIEGAND_USER_CODE_MAX];
                    snprintf(captured_code, sizeof(captured_code), "%s", ctx->bit_buffer);
                    wiegand_log_frame_hex(captured_code, ctx->bit_buffer_len, ctx->channel, false);
                    wiegand_process_code(ctx, captured_code);
                }
                wiegand_reset_bit_buffer(ctx);
                ctx->incomingCodeCount = 0;
                memset(ctx->incomingCode, 0, sizeof(ctx->incomingCode));
            }
        }
    }
}

void enableWiegand(int ch, bool val) {
    for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
        if (wg[i].channel == ch) {
            wg[i].enable = val;
        }
    }
}


void wiegand_main(void) {
	wg[0].pin0 = WG0_DATA0_IO;
	wg[0].pin1 = WG0_DATA1_IO;
	wg[0].pin_push = OPEN_IO_1;
	wg[0].delay = 4;
	wg[0].keypressTimeout = 4;
	wg[0].channel = 1;
	wg[0].enable = true;
	wg[0].alert = true;
	wg[0].newKey = false;
	wg[0].keypressExpired = true;  // Start as expired to prevent false timeout at boot
	wg[0].keypressCount = 0;
	wg[0].expired = true;
	wg[0].count = 0;
    memset(wg[0].incomingCode, 0, sizeof(wg[0].incomingCode));
    memset(wg[0].code, 0, sizeof(wg[0].code));
    wg[0].incomingCodeCount = 0;
    wiegand_reset_bit_buffer(&wg[0]);
	strcpy(wg[0].name, "Wiegand0");

	wg[1].pin0 = WG1_DATA0_IO;
	wg[1].pin1 = WG1_DATA1_IO;
	wg[1].pin_push = OPEN_IO_1;
	wg[1].delay = 4;
	wg[1].keypressTimeout = 4;
	wg[1].channel = 2;
	wg[1].enable = true;
	wg[1].alert = true;
	wg[1].newKey = false;
	wg[1].keypressExpired = true;  // Start as expired to prevent false timeout at boot
	wg[1].keypressCount = 0;
	wg[1].expired = true;
	wg[1].count = 0;
    memset(wg[1].incomingCode, 0, sizeof(wg[1].incomingCode));
    memset(wg[1].code, 0, sizeof(wg[1].code));
    wg[1].incomingCodeCount = 0;
    wiegand_reset_bit_buffer(&wg[1]);
	strcpy(wg[1].name, "Wiegand1");

    ESP_LOGI(LOG_TAG_WIEGAND, "Initializing Wiegand: WG0 (channel 1) GPIO%d/DATA0, GPIO%d/DATA1, enabled=%d",
             wg[0].pin0, wg[0].pin1, wg[0].enable);
    ESP_LOGI(LOG_TAG_WIEGAND, "Initializing Wiegand: WG1 (channel 2) GPIO%d/DATA0, GPIO%d/DATA1, enabled=%d",
             wg[1].pin0, wg[1].pin1, wg[1].enable);

    gpio_evt_queue = xQueueCreate(128, sizeof(gpio_event_t));  // Must hold 2x bits for ANYEDGE

	xTaskCreate(wiegand_timer, "wigand_timer", 3072, NULL, 10, NULL);
	xTaskCreate(keypress_timer, "keypress_timer", 3072, NULL, 10, NULL);
    xTaskCreate(wiegand_task, "wiegand_task", 4096, NULL, 10, NULL);

    for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
        ESP_LOGI(LOG_TAG_WIEGAND, "Registering ISR handlers for WG%d: GPIO%d and GPIO%d",
                 i, wg[i].pin0, wg[i].pin1);
        gpio_isr_handler_add(wg[i].pin0, wiegand_isr_handler, (void *)(uintptr_t)wg[i].pin0);
        gpio_isr_handler_add(wg[i].pin1, wiegand_isr_handler, (void *)(uintptr_t)wg[i].pin1);
	}
}

esp_err_t wiegand_registration_start(uint8_t channel) {
    if (channel > NUM_OF_WIEGANDS) {
        channel = 0;
    }

    esp_err_t result = ESP_OK;
    portENTER_CRITICAL(&registrationMutex);
    if (registration_session.active) {
        result = ESP_ERR_INVALID_STATE;
    } else {
        registration_clear_session_locked();
        registration_session.active = true;
        registration_session.channel = channel;
    }
    portEXIT_CRITICAL(&registrationMutex);

    if (result == ESP_OK) {
        ESP_LOGI(LOG_TAG_WIEGAND, "Wiegand registration started (channel=%u)", (unsigned)channel);
    }
    return result;
}

esp_err_t wiegand_registration_stop(bool promote_pending) {
    char ids[WIEGAND_SESSION_MAX][WIEGAND_USER_ID_MAX];
    memset(ids, 0, sizeof(ids));
    uint8_t channel = 0;
    bool was_active = false;

    size_t captured = registration_collect_ids(ids, WIEGAND_SESSION_MAX, &channel, &was_active);
    if (!was_active) {
        return ESP_ERR_INVALID_STATE;
    }

    if (promote_pending) {
        for (size_t i = 0; i < captured; i++) {
            if (ids[i][0] == '\0') {
                continue;
            }
            esp_err_t err = wiegand_registry_update_status(ids[i], WIEGAND_USER_STATUS_ACTIVE);
            if (err != ESP_OK) {
                ESP_LOGW(LOG_TAG_WIEGAND, "Failed to activate Wiegand user %s (%s)", ids[i], esp_err_to_name(err));
            }
        }
    }

    ESP_LOGI(LOG_TAG_WIEGAND, "Wiegand registration stopped; %u new codes captured", (unsigned)captured);
    return ESP_OK;
}

bool wiegand_registration_is_active(void) {
    return registration_snapshot(NULL, NULL);
}

uint8_t wiegand_registration_channel(void) {
    uint8_t channel = 0;
    registration_snapshot(&channel, NULL);
    return channel;
}

const char *wiegand_registration_last_duplicate(void) {
    static char duplicate[WIEGAND_USER_CODE_MAX];
    portENTER_CRITICAL(&registrationMutex);
    snprintf(duplicate, sizeof(duplicate), "%s", registration_session.last_duplicate);
    portEXIT_CRITICAL(&registrationMutex);
    return duplicate;
}

size_t wiegand_registration_pending_count(void) {
    return registration_pending_count();
}

cJSON *wiegand_state_snapshot(void) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    bool active = wiegand_registration_is_active();
    uint8_t channel = wiegand_registration_channel();
    size_t pending = registration_pending_count();
    const char *duplicate = wiegand_registration_last_duplicate();

    cJSON_AddBoolToObject(root, "registrationActive", active);
    cJSON_AddNumberToObject(root, "registrationChannel", channel);
    cJSON_AddNumberToObject(root, "registrationPending", (double)pending);
    if (duplicate && duplicate[0] != '\0') {
        cJSON_AddStringToObject(root, "lastDuplicateCode", duplicate);
    }

    cJSON *users = wiegand_registry_snapshot();
    if (!users) {
        users = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "users", users);
    return root;
}

