#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "xtensa/corebits.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "soc/cpu.h"

#define BINARY_SWITCH_IO     4
#define GPIO_INPUT_PIN_SEL  ((1ULL<<BINARY_SWITCH_IO))
#define ESP_INTR_FLAG_DEFAULT 0
#define DEBOUNCE_TIME 100

static xQueueHandle switch_evt_queue = NULL;
bool new_binary_switch_event = false;

bool debounce = false;
bool switch_event = false;
int switch_level = 0;

static void IRAM_ATTR binary_switch_isr_handler(void* arg)
{
    switch_event = true;
    switch_level = gpio_get_level(BINARY_SWITCH_IO);
}

int get_switch_level(){
    if (switch_event) switch_event = false;
    return switch_level;
}

static int binary_switch_event(){
    int ret = switch_event;
    return ret;
}

void binary_switch_main()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(BINARY_SWITCH_IO, GPIO_INTR_ANYEDGE);
    //create a queue to handle gpio event from isr
    switch_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BINARY_SWITCH_IO, binary_switch_isr_handler, (void*) BINARY_SWITCH_IO);
}
