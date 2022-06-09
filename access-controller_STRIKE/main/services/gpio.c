#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gpio.h"
#include <string.h>

#define MCP_INTA_GPIO 	34
#define MCP_INTB_GPIO 	35

#define LOCK_IO_1 	18
#define LOCK_IO_2 	19

#define CONTACT_IO_1 	15
#define CONTACT_IO_2 	16

#define OPEN_IO_1		21
#define OPEN_IO_2		23

#define EXIT_BUTTON_IO_1		14
#define EXIT_BUTTON_IO_2		14
#define KEYPAD_IO_1					13
#define KEYPAD_IO_2					27

#define BUZZER_IO 			22

#define WG0_DATA0_IO		27
#define WG0_DATA1_IO		26
#define WG1_DATA0_IO		32
#define WG1_DATA1_IO		5

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<BUZZER_IO) | (1ULL<<LOCK_IO_1) | (1ULL<<OPEN_IO_1))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<CONTACT_IO_1) | (1ULL<<CONTACT_IO_2) | (1ULL<<MCP_INTA_GPIO) | (1ULL<<MCP_INTB_GPIO) | (1ULL<<KEYPAD_IO_1))
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

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

bool get_io (uint8_t io)
{
	bool val = false;
	if (USE_MCP23017) {
		val = get_mcp_io(io);
	} else {
		val = gpio_get_level(io);
		// printf("get_io io:%d\tval: %d\n", io, val);
	}
	return val;
}

void set_io (uint8_t io, bool val)
{
	if (USE_MCP23017) {
		set_mcp_io(io, val);
	} else {
		printf("gpio_set_level io:%d\tval: %d\n", io, val);
		gpio_set_level(io, val);
	}
}
