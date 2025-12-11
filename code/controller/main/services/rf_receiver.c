/**
 * RF Receiver Driver for 433MHz RXB6 Module
 * 
 * Decodes EV1527/PT2262 protocol commonly used by 433MHz key fobs.
 * 
 * Protocol: 24-bit code transmitted as:
 *   - Sync: Long HIGH pulse (~11ms) followed by short LOW
 *   - Data: 24 bits, each bit is a HIGH+LOW pair
 *     - Bit 0: Short HIGH (~350us), Long LOW (~1100us)
 *     - Bit 1: Long HIGH (~1100us), Short LOW (~350us)
 *   - Ratio is approximately 3:1
 * 
 * The RXB6 outputs noise when idle - we filter this by requiring
 * a valid sync pulse before capturing data.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rf_registry.h"

/* GPIO pin for RF DATA input */
#define RF_DATA_GPIO        15

/* Timing thresholds (microseconds) */
#define RF_MIN_PULSE_US     80      /* Ignore noise spikes < 80us */
#define RF_MAX_PULSE_US     18000   /* Max reasonable pulse length */
#define RF_SYNC_MIN_US      9000    /* Min sync pulse length */
#define RF_SYNC_MAX_US      14000   /* Max sync pulse length */
#define RF_SILENCE_US       6000    /* Period of silence to detect signal end */
#define RF_DEBOUNCE_US      500000  /* 500ms debounce between same code */

/* Expected timing for EV1527/PT2262 (we will auto-tune per capture) */
#define RF_SHORT_MIN_US     180
#define RF_SHORT_MAX_US     750
#define RF_LONG_MIN_US      900
#define RF_LONG_MAX_US      1800
#define RF_RATIO_MIN        2.2f
#define RF_RATIO_MAX        4.5f
#define RF_MIN_VALID_PULSES 45
#define RF_MAX_VALID_PULSES 70

/* Capture buffer */
#define RF_CAPTURE_SIZE     128

static const char *RF_TAG = "rf_receiver";

/* Pulse capture buffer - STATIC to avoid stack usage */
typedef struct {
    uint16_t duration_us;
    uint8_t level;
} rf_pulse_t;

static rf_pulse_t rf_pulses[RF_CAPTURE_SIZE];
static volatile size_t rf_pulse_count = 0;
static volatile int64_t rf_last_edge_us = 0;
static volatile bool rf_capturing = false;
static volatile bool rf_sync_detected = false;

/* Debounce */
static uint32_t rf_last_code = 0;
static int64_t rf_last_code_time_us = 0;

/**
 * GPIO ISR handler - capture edges with sync detection
 */
static void IRAM_ATTR rf_isr_handler(void *arg)
{
    int64_t now_us = esp_timer_get_time();
    uint8_t current_level = gpio_get_level(RF_DATA_GPIO);
    
    if (rf_last_edge_us > 0) {
        uint32_t pulse_us = (uint32_t)(now_us - rf_last_edge_us);
        uint8_t prev_level = !current_level;
        
        /* Filter obvious noise */
        if (pulse_us < RF_MIN_PULSE_US || pulse_us > RF_MAX_PULSE_US) {
            rf_last_edge_us = now_us;
            return;
        }
        
        /* Look for sync pulse: Long HIGH followed by transition to LOW */
        if (!rf_capturing && prev_level == 1 && 
            pulse_us >= RF_SYNC_MIN_US && pulse_us <= RF_SYNC_MAX_US) {
            /* Valid sync pulse detected - start capturing */
            rf_capturing = true;
            rf_sync_detected = true;
            rf_pulse_count = 0;
            
            /* Record the sync pulse */
            rf_pulses[rf_pulse_count].duration_us = (uint16_t)pulse_us;
            rf_pulses[rf_pulse_count].level = prev_level;
            rf_pulse_count++;
        }
        else if (rf_capturing && rf_pulse_count < RF_CAPTURE_SIZE) {
            /* Capture data pulses after sync */
            rf_pulses[rf_pulse_count].duration_us = (pulse_us > 65535) ? 65535 : (uint16_t)pulse_us;
            rf_pulses[rf_pulse_count].level = prev_level;
            rf_pulse_count++;
        }
    }
    
    rf_last_edge_us = now_us;
}

/**
 * Check if a pulse duration is "short" (~300-500us)
 */
static inline bool is_short_pulse(uint16_t us, uint16_t short_us)
{
    /* Accept within +/-30% of measured short_us */
    uint16_t min = (uint16_t)(short_us * 7 / 10);
    uint16_t max = (uint16_t)(short_us * 13 / 10);
    return us >= min && us <= max;
}

static inline bool is_long_pulse(uint16_t us, uint16_t long_us)
{
    /* Accept within +/-30% of measured long_us */
    uint16_t min = (uint16_t)(long_us * 7 / 10);
    uint16_t max = (uint16_t)(long_us * 13 / 10);
    return us >= min && us <= max;
}

/**
 * Try to decode captured pulses as EV1527/PT2262
 * Returns 24-bit code or 0 if invalid
 */
static uint32_t rf_try_decode(size_t count)
{
    /* Need sync + at least 48 pulses (24 bit pairs) */
    if (count < 49) {
        return 0;
    }
    
    /* First pulse should be sync (already validated in ISR, but double-check) */
    if (rf_pulses[0].level != 1 || rf_pulses[0].duration_us < RF_SYNC_MIN_US || rf_pulses[0].duration_us > RF_SYNC_MAX_US) {
        return 0;
    }

    /* Estimate short/long from the following pulses */
    uint32_t short_sum = 0, long_sum = 0;
    int short_cnt = 0, long_cnt = 0;

    /* Use up to the first 60 pulses after sync to estimate */
    size_t max_estimate = (count > 61) ? 61 : count;
    for (size_t i = 1; i < max_estimate; i++) {
        uint16_t d = rf_pulses[i].duration_us;
        if (d >= RF_SHORT_MIN_US && d <= RF_SHORT_MAX_US) {
            short_sum += d;
            short_cnt++;
        } else if (d >= RF_LONG_MIN_US && d <= RF_LONG_MAX_US) {
            long_sum += d;
            long_cnt++;
        }
    }

    if (short_cnt < 5 || long_cnt < 5) {
        /* Not enough data to estimate */
        return 0;
    }

    uint16_t short_us = (uint16_t)(short_sum / short_cnt);
    uint16_t long_us  = (uint16_t)(long_sum  / long_cnt);

    /* Validate ratio */
    float ratio = (float)long_us / (float)short_us;
    if (ratio < RF_RATIO_MIN || ratio > RF_RATIO_MAX) {
        return 0;
    }

    /* Decode 24 bits starting after sync */
    uint32_t code = 0;
    int bits = 0;
    int errors = 0;
    for (size_t i = 1; i + 1 < count && bits < 24; i += 2) {
        uint16_t first_us = rf_pulses[i].duration_us;
        uint16_t second_us = rf_pulses[i + 1].duration_us;

        int bit = -1;

        /* Bit 0: Short + Long */
        if (is_short_pulse(first_us, short_us) && is_long_pulse(second_us, long_us)) {
            bit = 0;
        }
        /* Bit 1: Long + Short */
        else if (is_long_pulse(first_us, long_us) && is_short_pulse(second_us, short_us)) {
            bit = 1;
        }
        /* Fallback: ratio-based check */
        else if (first_us < second_us && (float)second_us / (float)first_us > (RF_RATIO_MIN - 0.2f)) {
            bit = 0;
        }
        else if (first_us > second_us && (float)first_us / (float)second_us > (RF_RATIO_MIN - 0.2f)) {
            bit = 1;
        }

        if (bit >= 0) {
            code = (code << 1) | bit;
            bits++;
        } else {
            errors++;
            if (errors > 2) {
                return 0;
            }
        }
    }

    return (bits == 24) ? code : 0;
}

/**
 * Process captured RF signals - runs in task context
 */
static void rf_process_task(void *pvParameter)
{
    ESP_LOGI(RF_TAG, "RF receiver task started on GPIO%d", RF_DATA_GPIO);
    ESP_LOGI(RF_TAG, "Sync: %d-%dus, Short: %d-%dus, Long: %d-%dus",
             RF_SYNC_MIN_US, RF_SYNC_MAX_US,
             RF_SHORT_MIN_US, RF_SHORT_MAX_US,
             RF_LONG_MIN_US, RF_LONG_MAX_US);
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(15));
        
        int64_t now_us = esp_timer_get_time();
        
        /* Check for signal end (silence) */
        if (rf_capturing && rf_pulse_count > 0) {
            int64_t since_last = now_us - rf_last_edge_us;
            
            if (since_last > RF_SILENCE_US) {
                /* Signal ended - process capture */
                size_t count = rf_pulse_count;
                bool had_sync = rf_sync_detected;
                
                /* Stop capturing */
                rf_capturing = false;
                rf_sync_detected = false;
                rf_pulse_count = 0;
                
                /* Only process if we had a valid sync and pulse count in expected window */
                if (had_sync && count >= 49 && count >= RF_MIN_VALID_PULSES && count <= RF_MAX_VALID_PULSES) {
                    uint32_t code = rf_try_decode(count);
                    
                    if (code != 0) {
                        /* Debounce: ignore if same code within 500ms */
                        bool is_duplicate = (code == rf_last_code) && 
                                           ((now_us - rf_last_code_time_us) < RF_DEBOUNCE_US);
                        
                        if (!is_duplicate) {
                            /* NEW VALID CODE! */
                            ESP_LOGI(RF_TAG, "========================================");
                            ESP_LOGI(RF_TAG, "RF CODE RECEIVED: 0x%06lX", (unsigned long)code);
                            ESP_LOGI(RF_TAG, "  Full 24-bit code: 0x%06lX (%lu)", 
                                     (unsigned long)code, (unsigned long)code);
                            ESP_LOGI(RF_TAG, "  Binary: %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d",
                                     (code >> 23) & 1, (code >> 22) & 1, (code >> 21) & 1, (code >> 20) & 1,
                                     (code >> 19) & 1, (code >> 18) & 1, (code >> 17) & 1, (code >> 16) & 1,
                                     (code >> 15) & 1, (code >> 14) & 1, (code >> 13) & 1, (code >> 12) & 1,
                                     (code >> 11) & 1, (code >> 10) & 1, (code >> 9) & 1, (code >> 8) & 1,
                                     (code >> 7) & 1, (code >> 6) & 1, (code >> 5) & 1, (code >> 4) & 1,
                                     (code >> 3) & 1, (code >> 2) & 1, (code >> 1) & 1, (code >> 0) & 1);
                            ESP_LOGI(RF_TAG, "  Pulses captured: %d", (int)count);
                            ESP_LOGI(RF_TAG, "========================================");
                            
                            rf_last_code = code;
                            rf_last_code_time_us = now_us;
                            
                            /* Registration path vs live action */
                            if (rf_registry_is_active()) {
                                rf_registry_on_code(code, count);
                            } else {
                                rf_registry_handle_code(code);
                            }
                        }
                    } else {
                        ESP_LOGD(RF_TAG, "Decode failed (count=%d, sync=%d)", (int)count, had_sync);
                    }
                } else {
                    ESP_LOGD(RF_TAG, "Discard capture (sync=%d, count=%d)", had_sync, (int)count);
                }
            }
        }
        
        /* Reset if buffer full (shouldn't happen with sync detection) */
        if (rf_pulse_count >= RF_CAPTURE_SIZE - 2) {
            rf_capturing = false;
            rf_sync_detected = false;
            rf_pulse_count = 0;
        }
    }
}

/**
 * Initialize the RF receiver
 */
void rf_receiver_init(void)
{
    ESP_LOGI(RF_TAG, "Initializing RF receiver on GPIO%d", RF_DATA_GPIO);
    
    /* Configure GPIO as input with pull-up, interrupt on any edge */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RF_DATA_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(RF_TAG, "GPIO%d initial level: %d", RF_DATA_GPIO, gpio_get_level(RF_DATA_GPIO));
    
    /* Install ISR handler */
    gpio_isr_handler_add(RF_DATA_GPIO, rf_isr_handler, NULL);
    
    /* Start processing task */
    xTaskCreate(rf_process_task, "rf_rx", 8192, NULL, 5, NULL);
    
    ESP_LOGI(RF_TAG, "RF receiver ready - waiting for fob signals...");
}

/**
 * Get the last received RF code
 */
uint32_t rf_get_last_code(void)
{
    return rf_last_code;
}
