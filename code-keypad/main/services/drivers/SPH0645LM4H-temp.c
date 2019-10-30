#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "xtensa/corebits.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "soc/cpu.h"

#include <stdio.h>
#include "esp_types.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#define PIR_FREQ_KHZ (32)
#define PIR_PERIOD_US ((uint64_t)(1000000.0 / (PIR_FREQ_KHZ * 1000)))
#define PIR_IO (GPIO_NUM_15)
#define PIR_BITS (42)
#define CPU_FREQ_MHZ (240)
#define CPU_TICK_US (1000000.0 / (CPU_FREQ_MHZ * 1000000))
#define OUTPUT_MODE (1)
#define INPUT_MODE (0)

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (3.4179) // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC   (5.78)   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD 1 // testing will be done with auto reload

#define PIR_TIMER_INTERVAL   (0.015625) // mic_test timer to get 64 Hz

xQueueHandle timer_queue;

inline void busy_delay_us(float us) {
  uint32_t t0_ccount, t1_ccount;
  t0_ccount = CCOUNT;
  while (true) {
    RSR(CCOUNT, t1_ccount);
    if (t1_ccount - t0_ccount > ((uint32_t)(us / CPU_TICK_US))) {
      break;
    }
  }
}

void set_gpio_mode (int mode) {

  if (mode == OUTPUT_MODE) {
    gpio_config_t c = {0};
    c.pin_bit_mask = 1 << PIR_IO;
    c.mode = GPIO_MODE_OUTPUT; // start here to reset to 0
    c.pull_down_en = GPIO_PULLDOWN_ENABLE;
    c.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&c);
  }

  if (mode == INPUT_MODE) {
    gpio_config_t c = {0};
    c.pin_bit_mask = 1 << PIR_IO;
    c.mode = GPIO_MODE_INPUT; // start here to reset to 0
    c.pull_down_en = GPIO_PULLDOWN_ENABLE;
    c.pull_up_en = GPIO_PULLUP_DISABLE;
    //c.intr_type = GPIO_PIN_INTR_POSEDGE;
    gpio_config(&c);
  }

}

void IRAM_ATTR mic_test_timer_isr(void *para) {
    int timer_idx = (int) para;
    TIMERG1.int_clr_timers.t1 = 1;
    TIMERG1.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
    pir_timer_count++;
}

static void mic_test_timer_init(int timer_idx, double timer_interval_sec) {
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 1;
    timer_init(TIMER_GROUP_1, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_1, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_1, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_1, timer_idx);
    timer_isr_register(TIMER_GROUP_1, timer_idx, mic_test_timer_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_1, timer_idx);
}

static void mic_test_gpio_init() {
      gpio_config_t c = {0};
      c.pin_bit_mask = 1 << PIR_IO;
      c.mode = GPIO_MODE_OUTPUT; // start here to reset to 0
      c.pull_down_en = GPIO_PULLDOWN_ENABLE;
      c.pull_up_en = GPIO_PULLUP_DISABLE;
      c.intr_type = GPIO_PIN_INTR_DISABLE;
      gpio_config(&c);

      gpio_set_level(PIR_IO, 0);
}

void task_mic_test(void * param) {
    esp_timer_handle_t t = NULL;
    esp_err_t result;

    mic_test_gpio_init();
    mic_test_timer_init(TIMER_1, PIR_TIMER_INTERVAL);

    while (1) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      printf("Motion Level:\t%d\tSample Rate:\t%d\n", motion_level, mic_test_timer_count * 2);
      mic_test_timer_count = 0;
    }
}

void mic_test_main() {
    xTaskCreate(task_mic_test, "PIR_TIMER", 2048, NULL, 10, NULL);
}
