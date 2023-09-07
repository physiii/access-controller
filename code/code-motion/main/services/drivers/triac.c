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

#define TRIAC_IO    2
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<TRIAC_IO))
#define ZERO_DETECT_IO     35
#define GPIO_INPUT_PIN_SEL  ((1ULL<<ZERO_DETECT_IO))
#define ESP_INTR_FLAG_DEFAULT 0
#define TRIAC_OFF 0
#define TRIAC_ON 1

#define CPU_FREQ_MHZ (240)
#define CPU_TICK_US (1000000.0 / (CPU_FREQ_MHZ * 1000000))

int triac_off_time = 1;
int zero_cross_count = 0;
int cnt = 0;
int level = 0;

void delay_triac_us(int us) {
  uint32_t t0_ccount, t1_ccount;
  t0_ccount = CCOUNT;
  gpio_set_level(TRIAC_IO, TRIAC_OFF);
  while (true) {
    RSR(CCOUNT, t1_ccount);
    if (t1_ccount - t0_ccount > ((uint32_t)(us / CPU_TICK_US))) {
      break;
    }
  }
  gpio_set_level(TRIAC_IO, TRIAC_ON);
}

static void IRAM_ATTR zero_cross_isr_handler(void* arg) {
    zero_cross_count++;
    //level = gpio_get_level(gpio_num);
    delay_triac_us(50 * 1000);
}

static void triac_task(void* arg)
{
    uint32_t io_num;
    gpio_set_level(TRIAC_IO, TRIAC_ON);
    printf("starting triac driver\n");
    for(;;) {
        //printf("%d delay %d zeros: %d\n", cnt++,triac_off_time, zero_cross_count);
        zero_cross_count = 0;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void triac_main()
{
    // gpio_config_t io_conf;
    // io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // io_conf.pull_down_en = 0;
    // io_conf.pull_up_en = 0;
    // gpio_config(&io_conf);
    //
    // io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_up_en = 0;
    // gpio_config(&io_conf);
    //
    // gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // gpio_isr_handler_add(ZERO_DETECT_IO, zero_cross_isr_handler, (void*) ZERO_DETECT_IO);

    xTaskCreate(triac_task, "triac_task", 2048, NULL, 10, NULL);
}
