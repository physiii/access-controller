// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "driver/gpio.h"

#define WG_DELAY	100

struct wiegand
{
	uint8_t pin0;
	uint8_t pin1;
	uint64_t msg;
	char name[100];
	bool enable;
};

struct wiegand wg0;
struct wiegand wg1;


static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR wiegand_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;

		if (gpio_num == wg0.pin0) {
			wg0.msg = (wg0.msg << 1) | 0;
		} else if (gpio_num == wg0.pin1) {
			wg0.msg = (wg0.msg << 1) | 1;
		}
}


static void wiegand_task(void *pvParameter) {
	uint32_t io_num;
	for(;;) {
    if (wg0.msg != 0) {
			vTaskDelay(WG_DELAY / portTICK_PERIOD_MS);
			printf("%s: %lld\n", wg0.name, wg0.msg);
			wg0.msg = 0;
		} else {
	  	vTaskDelay(WG_DELAY / portTICK_PERIOD_MS);
		}
	}
}

void enable_wiegand(bool val) {
	wg0.enable = val;
}

void wiegand_main() {
	wg0.pin0 = CONTACT_IO_1;
	wg0.pin1 = CONTACT_IO_2;
	wg0.enable = true;
	strcpy(wg0.name, "Wiegand0");

	wg1.pin0 = WG1_DATA0_IO;
	wg1.pin1 = WG1_DATA1_IO;
	wg1.enable = false;

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

  xTaskCreate(wiegand_task, "wiegand_task", 2048, NULL, 10, NULL);

  gpio_isr_handler_add(wg0.pin0, wiegand_isr_handler, (void*) wg0.pin0);
  gpio_isr_handler_add(wg0.pin1, wiegand_isr_handler, (void*) wg0.pin1);

  // gpio_isr_handler_add(WG0_DATA0_IO, wiegand_isr_handler, (void*) WG0_DATA0_IO);
  // gpio_isr_handler_add(WG0_DATA1_IO, wiegand_isr_handler, (void*) WG0_DATA1_IO);
  // gpio_isr_handler_add(WG1_DATA0_IO, wiegand_isr_handler, (void*) WG1_DATA0_IO);
  // gpio_isr_handler_add(WG1_DATA1_IO, wiegand_isr_handler, (void*) WG1_DATA1_IO);
}
