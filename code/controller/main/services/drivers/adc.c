#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          // Multisampling

static adc_oneshot_unit_handle_t adc1_handle;
static const adc_channel_t channel = ADC_CHANNEL_7; // GPIO34 if ADC1, GPIO14 if ADC2

void check_efuse(void) {
    // Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

void adc_init(void) {
    // Configure ADC
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_6,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc1_handle, channel, &chan_config);
}

static void adc_task(void* arg) {
    int adc_reading = 0;
    while (1) {
        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_oneshot_read(adc1_handle, channel, &adc_reading);
        }
        printf("ADC Reading: %d\n", adc_reading);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void adc_main(void) {
    // Check if Two Point or Vref are burned into eFuse
    check_efuse();

    adc_init();

    xTaskCreate(adc_task, "adc_task", 2048, NULL, 10, NULL);
}
