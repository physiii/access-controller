#include "gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "automation.h"
#include "drivers/mcp23x17.h"

void gpio_main(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

bool get_io(uint8_t io) {
    bool val;
    if (USE_MCP23017) {
        val = get_mcp_io(io);
    } else {
        val = gpio_get_level(io);
    }
    return val;
}

void set_io(uint8_t io, bool val) {
    if (USE_MCP23017) {
        set_mcp_io(io, val);
    } else {
        gpio_set_level(io, val);
    }
}
