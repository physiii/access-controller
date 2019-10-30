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
#define PIR_TIMER_INTERVAL   (0.015625) // pir timer to get 64 Hz

int motion_threshold = 1500;
int max_motion_level = 5000;
int motion_scale = 255;
int motion_bias = 100;
int motion_state = 0;
int motion_level;
double alpha = 0.1;
double alpha_avg = 0.001;
int bandpass_low = 3000;
int bandpass_high = 14000;
float delta_bandpass_low = 0.535;
float delta_bandpass_high = 0.9;
int pir_bits_remaining; // set when timer starts, decremented in handler
uint64_t pir_bits; // 0-41 used; 42-63 unused
uint64_t t0, t1;
bool data_tx_started = false;
int channel, bit;
int accumulator = 8000;
int accumulator_avg = 8000;
int count = 0;
int pir_timer_count = 0;
int mic_test_timer_count = 0;

xQueueHandle timer_queue;

struct pir_frame_t {
  uint16_t channel[3];
};
struct pir_frame_t frame = {0};
struct pir_frame_t previous_frame = {0};
struct pir_frame_t delta_frame = {0};
struct pir_frame_t previous_delta_frame = {0};
struct pir_frame_t accumulator_frame = {0};
struct pir_frame_t average_frame = {0};

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

bool get_motion_state() {
  int state = motion_state;
  motion_state = false;
  return state;
}

void set_motion_threshold(int sensitivity) {
  motion_threshold = motion_bias + (max_motion_level - motion_bias) * sensitivity / motion_scale;
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

void IRAM_ATTR fill_pir_frame() {
  // wait DL high
  gpio_set_direction(PIR_IO, GPIO_MODE_INPUT);
  while (0 == gpio_get_level(PIR_IO));

  // wait 25us for setup
  busy_delay_us(25.0);

  for (channel = 0; channel < 3; channel++) {
    frame.channel[channel] = 0;
    for (bit = 0; bit < 14; bit += 1) {

      //pir expects low to high transition to get each bit
      gpio_set_direction(PIR_IO, GPIO_MODE_OUTPUT);
      gpio_set_level(PIR_IO, 0);
      busy_delay_us(0.2);
      gpio_set_level(PIR_IO, 1);
      busy_delay_us(0.2);

      // shift frame by one for next bit
      frame.channel[channel] <<= 1;
      gpio_set_direction(PIR_IO, GPIO_MODE_INPUT);

      // wait 5us for bit to settle
      busy_delay_us(5);

      // read bit and put into frame
      if (gpio_get_level(PIR_IO)) frame.channel[channel] |= 1;
    }
  }

  // bandpass filter on raw data
  if (frame.channel[0] > bandpass_low && frame.channel[0] < bandpass_high) {
    accumulator = (alpha * frame.channel[0]) + (1.0 - alpha) * accumulator;
    accumulator_avg = (alpha_avg * frame.channel[0]) + (1.0 - alpha_avg) * accumulator_avg;
    motion_level = accumulator - accumulator_avg;
    if (motion_level < 0) motion_level = 0 - motion_level;
    if (motion_level > motion_threshold) motion_state = 1;
  }
}

void IRAM_ATTR timer_group0_isr(void *para) {
  
  int timer_idx = (int) para;
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  TIMERG0.hw_timer[timer_idx].update = 1;
  uint64_t timer_counter_value =
    ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
    | TIMERG0.hw_timer[timer_idx].cnt_low;

  if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
    TIMERG0.int_clr_timers.t1 = 1;
    if (!storage_in_use) fill_pir_frame();
    pir_timer_count++;
  }

  TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
}

static void tg0_timer_init(int timer_idx, double timer_interval_sec) {
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 1;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

static void pir_gpio_init() {
      gpio_config_t c = {0};
      c.pin_bit_mask = 1 << PIR_IO;
      c.mode = GPIO_MODE_OUTPUT; // start here to reset to 0
      c.pull_down_en = GPIO_PULLDOWN_ENABLE;
      c.pull_up_en = GPIO_PULLUP_DISABLE;
      c.intr_type = GPIO_PIN_INTR_DISABLE;
      gpio_config(&c);

      gpio_set_level(PIR_IO, 0);
}

void task_pir(void * param) {
    esp_timer_handle_t t = NULL;
    esp_err_t result;

    pir_gpio_init();
    tg0_timer_init(TIMER_1, PIR_TIMER_INTERVAL);

    while (1) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      // printf("PIR Level:\t%d\tPIR Sample Rate:\t%d\n", motion_level, pir_timer_count);
      pir_timer_count = 0;
    }
}

void pir_main() {
    xTaskCreatePinnedToCore(&task_pir, "PIR_TIMER", 16384, NULL, 20, NULL, 0);
}
