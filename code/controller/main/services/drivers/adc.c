#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_7;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_6;
static const adc_unit_t unit = ADC_UNIT_1;

uint32_t adc5_reading = 0;
uint32_t adc6_reading = 0;
uint32_t adc7_reading = 0;

static void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void adc_init (adc_channel_t channel) {
    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

static void adc_task(void* arg)
{
	while (1) {
			//Multisampling
			for (int i = 0; i < NO_OF_SAMPLES; i++) {
					if (unit == ADC_UNIT_1) {
							adc5_reading += adc1_get_raw((adc1_channel_t)ADC_CHANNEL_5);
							adc6_reading += adc1_get_raw((adc1_channel_t)ADC_CHANNEL_6);
							adc7_reading += adc1_get_raw((adc1_channel_t)ADC_CHANNEL_7);
					}
			}
			adc5_reading /= NO_OF_SAMPLES;
			adc6_reading /= NO_OF_SAMPLES;
			adc7_reading /= NO_OF_SAMPLES;
			//Convert adc_reading to voltage in mV
			// uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
			// printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
			// printf("adc5: %d\tadc6: %d\tadc7: %d\n", adc5_reading, adc6_reading, adc7_reading);
			vTaskDelay(pdMS_TO_TICKS(100));
	}
}
void adc_main(void)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

		adc_init(ADC_CHANNEL_5);
		adc_init(ADC_CHANNEL_6);
		adc_init(ADC_CHANNEL_7);

		xTaskCreate(adc_task, "adc_task", 2048, NULL, 10, NULL);
}
