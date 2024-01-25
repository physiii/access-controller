#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gpio.h"
#include <string.h>

#define Y1_IO 		2
#define Y2_IO 		3
#define W1_IO 		4
#define W2_IO 		5
#define AUX1_IO		6
#define AUX2_IO		7

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<Y1_IO) | (1ULL<<Y2_IO) | (1ULL<<W1_IO) | (1ULL<<W2_IO))
// #define GPIO_INPUT_PIN_SEL  ((1ULL<<WG0_DATA0_IO) | (1ULL<<WG0_DATA1_IO) | (1ULL<<CONTACT_IO_1) | (1ULL<<CONTACT_IO_2) | (1ULL<<MCP_INTA_GPIO) | (1ULL<<MCP_INTB_GPIO) | (1ULL<<KEYPAD_IO_1) | (1ULL<<EXIT_BUTTON_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

void gpio_main(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // io_conf.intr_type = GPIO_INTR_ANYEDGE;
    // io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_up_en = 1;
    // gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

void set_io (uint8_t io, bool val)
{
	// printf("gpio_set_level io:%d\tval: %d\n", io, val);
	gpio_set_level(io, val);
}