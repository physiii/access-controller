// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "driver/gpio.h"

struct wiegand
{
	uint8_t pin0;
	uint8_t pin1;
	bool enable;
};

struct wiegand wg0;
struct wiegand wg1;

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR wiegand_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}


static void wiegand_task(void *pvParameter) {
	uint32_t io_num;
	for(;;) {
			if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
					printf("GPIO[%ld] intr, val: %d\n", io_num, gpio_get_level(io_num));
			}
	}
}

void enable_wiegand(bool val) {
	wg0.enable = val;
}

void wiegand_main() {
	wg0.pin0 = WG0_DATA0_IO;
	wg0.pin1 = WG0_DATA1_IO;
	wg0.enable = true;

	wg1.pin0 = WG1_DATA0_IO;
	wg1.pin1 = WG1_DATA1_IO;
	wg1.enable = true;

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

  xTaskCreate(wiegand_task, "wiegand_task", 2048, NULL, 10, NULL);

  gpio_isr_handler_add(WG0_DATA0_IO, wiegand_isr_handler, (void*) WG0_DATA0_IO);
  gpio_isr_handler_add(WG0_DATA1_IO, wiegand_isr_handler, (void*) WG0_DATA1_IO);
  gpio_isr_handler_add(WG1_DATA0_IO, wiegand_isr_handler, (void*) WG1_DATA0_IO);
  gpio_isr_handler_add(WG1_DATA1_IO, wiegand_isr_handler, (void*) WG1_DATA1_IO);
}
