#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "mcp23x17/mcp23x17.c"
#include <driver/gpio.h>
#include <string.h>

#define A0	0
#define A1	1
#define A2	2
#define A3	3
#define A4	4
#define A5	5
#define A6	6
#define A7	7

#define B0	8
#define B1	9
#define B2	10
#define B3	11
#define B4	12
#define B5	13
#define B6	14
#define B7	15

bool INTA = false;
bool INTB = false;

mcp23x17_t mcp_dev;

static void IRAM_ATTR mcp_inta_handler(void *arg)
{
    INTA = true;
}

static void IRAM_ATTR mcp_intb_handler(void *arg)
{
    INTB = true;
}

void mcp23017_task(void *pvParameters)
{
    memset(&mcp_dev, 0, sizeof(mcp23x17_t));
    ESP_ERROR_CHECK(mcp23x17_init_desc(&mcp_dev, 0, MCP23X17_ADDR_BASE, SDA_GPIO, SCL_GPIO));

    gpio_set_direction(MCP_INTA_GPIO, GPIO_MODE_INPUT);
		gpio_set_direction(MCP_INTB_GPIO, GPIO_MODE_INPUT);

    gpio_set_intr_type(MCP_INTA_GPIO, GPIO_INTR_ANYEDGE);
		gpio_set_intr_type(MCP_INTB_GPIO, GPIO_INTR_ANYEDGE);

    gpio_isr_handler_add(MCP_INTA_GPIO, mcp_inta_handler, NULL);
    gpio_isr_handler_add(MCP_INTB_GPIO, mcp_intb_handler, NULL);

    while (1)
    {
		    // printf("INTA %d\tINTB %d\n", INTA, INTB);
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void mcp23017_main()
{
    xTaskCreate(mcp23017_task, "mcp23017_task", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
}
