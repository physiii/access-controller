#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define MOTION_IO     15
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_IO_0     15
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0))

bool motion_active = false;

static xQueueHandle gpio_evt_queue = NULL;

bool get_motion_state() {
  int state = motion_active;
  return state;
}

static void IRAM_ATTR motion_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void pir_service(void* arg) {
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (gpio_get_level(io_num) == 0) {
              motion_active = false;
            } else {
              motion_active = true;
            }
        }
    }
}

void pir_main() {
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(pir_service, "pir_service_task", 2048, NULL, 10, NULL);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(MOTION_IO, motion_isr_handler, (void*) MOTION_IO);
}
