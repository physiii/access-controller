#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s.h"

const int sample_rate = 44100;
int mic_timer_count = 0;
uint16_t buf_len = 1024;
int32_t microphone_level = 0;

int get_microphone_level() {
	return microphone_level;
}

static void mic_gpio_init() {
  i2s_config_t i2s_config_rx = {
		.mode = I2S_MODE_MASTER | I2S_MODE_RX,
		.sample_rate = sample_rate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		// .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.dma_buf_count = 32,                            // number of buffers, 128 max.
		.dma_buf_len = 32 * 2,                          // size of each buffer
		.use_apll = 0,
		// .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1        // Interrupt level 1
		.intr_alloc_flags = ESP_INTR_FLAG_INTRDISABLED
	};

	i2s_pin_config_t pin_config_rx = {
		.bck_io_num = GPIO_NUM_26,
		.ws_io_num = GPIO_NUM_23,
		.data_out_num = I2S_PIN_NO_CHANGE,
		.data_in_num = GPIO_NUM_22
	};

	i2s_driver_install(I2S_NUM_1, &i2s_config_rx, 0, NULL);
	i2s_set_pin(I2S_NUM_1, &pin_config_rx);

	i2s_stop(I2S_NUM_1);
	i2s_start(I2S_NUM_1);

	i2s_driver_install(I2S_NUM_1, &i2s_config_rx, 0, NULL);
	i2s_set_pin(I2S_NUM_1, &pin_config_rx);
}

void task_microphone(void *pvParams) {
	char *buf = calloc(buf_len, sizeof(char));

	struct timeval tv = {0};
	struct timezone *tz = {0};
	gettimeofday(&tv, &tz);
	int bytes_written = 0;
	uint32_t cnt = 0;
	uint64_t micros = tv.tv_usec + tv.tv_sec * 1000000;
	uint64_t sample_time = tv.tv_sec;
	uint64_t prev_sample_time = tv.tv_sec;
	uint64_t micros_prev = micros;
	uint64_t delta = 0;
	uint32_t magnitude = 0;
	uint32_t magnitude_avg = 4294965610;
	uint32_t sample_sum = 0;
	uint32_t sample_min = 0;
	uint32_t sample_max = 0;
	double alpha_avg = 0.1;
	double alpha = 0.1;
	size_t bytes_read = 0;
	esp_err_t err = ESP_OK;

	mic_gpio_init();

	while(1) {

		char *buf_ptr_read = buf;
		bytes_read = 0;

		while(bytes_read == 0) err = i2s_read(I2S_NUM_1, buf, buf_len, &bytes_read, 2048);
		uint32_t samples_read = bytes_read / 2 / (I2S_BITS_PER_SAMPLE_32BIT / 8);

		for(int i = 0; i < samples_read; i++) {
			int16_t sample = (buf_ptr_read[3] << 8) + buf_ptr_read[2];
			buf_ptr_read += 2 * (I2S_BITS_PER_SAMPLE_32BIT / 8);
			if (sample > sample_max) sample_max = sample;
			if (sample < sample_min) sample_min = sample;
		}

		// magnitude = (alpha * sample) + (1.0 - alpha) * magnitude;
		// magnitude_avg = (alpha_avg * sample) + (1.0 - alpha) * magnitude_avg;
		cnt += samples_read;
		if (cnt >= sample_rate) {
			gettimeofday(&tv, &tz);
			sample_time = tv.tv_usec + tv.tv_sec * 1000000;
			// exponential moving average
			magnitude = sample_max - sample_min;
			magnitude_avg = (alpha_avg * magnitude) + (1.0 - alpha_avg) * magnitude_avg;
			if (magnitude > magnitude_avg) {
				microphone_level = magnitude - magnitude_avg;
			} else {
				microphone_level = magnitude_avg - magnitude;
			}

			// if (microphone_level > max_microphone_level) max_microphone_level = microphone_level;
			// printf("Avg:\t%u\tLevel:\t%u\tPeriod:\t%llu usec\n", magnitude_avg, microphone_level, sample_time - prev_sample_time);
			prev_sample_time = sample_time;
			// magnitude_avg = 0;
			magnitude = 0;
			sample_max = 0;
			sample_min = 0;
			cnt = 0;
		}
	}
}

void SPH0645LM4H_main() {
    printf("starting SPH0645LM4H_main()\n");
    xTaskCreatePinnedToCore(&task_microphone, "task_microphone", 16384, NULL, 20, NULL, 0);
}
