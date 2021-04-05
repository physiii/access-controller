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

#define BUZZER_IO 			14

#define WG0_DATA0_IO		16
#define WG0_DATA1_IO		4
#define WG1_DATA0_IO		32
#define WG1_DATA1_IO		5

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<BUZZER_IO))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<MCP_INTA_GPIO) | (1ULL<<MCP_INTB_GPIO))
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
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}
