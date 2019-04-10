#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "cJSON.h"
#include <stdlib.h>
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#define ZERO_DETECT_IO        35
#define TRIAC_IO              2

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

#define TRIAC_ON              1
#define TRIAC_OFF             0
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;
#define GPIO_INPUT_PIN_SEL  ((1ULL<<ZERO_DETECT_IO))
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<TRIAC_IO))

int NEUTRAL_PRESENT = 1;

int max_brightness = 255;
int zerocross_count = 0;
int current_brightness = 0;
double triac_delay = 1;
double min_triac_delay = 0.0001;
double max_triac_delay = 0.0075;
double no_neutral_thresh = 0.0040;

bool zerocross_present = false;
double sim_zerocross_delay = 0.0166; //simulated zero cross delay (1/120hz)
char dimmer_service_message[2000];
char dimmer_service_message_in[2000];

typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

xQueueHandle timer_queue;

// static void tg1_timer_init(int timer_idx,
//     bool auto_reload, double timer_interval_sec);

// sets delay from when zerocross is detected and
// triac is turned off to when the triac is turned
// on. A longer delay means less power available.
int set_brightness(int level) {

  // debounce the pir sensor when changing light values
  // really should communicate with motion service not pir
  // debounce_pir(10);

  if (level > max_brightness) level = max_brightness;
  if (level < 0) level = 0;

  if (!NEUTRAL_PRESENT) {
    if (triac_delay<no_neutral_thresh) {
      zerocross_present = false;
    } else if (!zerocross_present) {
      //printf("turning off triac to get zerocross %f\n",triac_delay);
      gpio_set_level(TRIAC_IO, TRIAC_OFF);
    }
  }

  if (level > 0 && level < max_brightness) {
    int triac_level = max_brightness - level;
    triac_delay = min_triac_delay + max_triac_delay*((double)triac_level/max_brightness);
    setLED(triac_level, triac_level, triac_level);
  }

  if (level == 0) {
    gpio_set_level(TRIAC_IO, TRIAC_OFF);
    setLED(255, 0, 0);
  }

  if (level == max_brightness) {
    gpio_set_level(TRIAC_IO, TRIAC_ON);
    setLED(0, 255, 0);
  }

  printf("Set brightness to %d.\n", level);
  return level;
}

void dec_brightness(int amount) {
  int new_brightness = current_brightness - amount;
  current_brightness = set_brightness(new_brightness);
}

void inc_brightness(int amount) {
  int new_brightness = current_brightness + amount;
  current_brightness = set_brightness(new_brightness);
}

void fade_brightness(int start, int stop, int interval) {
  printf("fading from %d to %d in %dms\n",start,stop,interval);
  int divider = 3;
  interval*=divider;
  if (stop>start) {
    interval = interval / (stop - start);
    for(int i=start; i<=stop; i+=divider) {
      current_brightness = set_brightness(i);
      if (interval<10) interval=10;
      vTaskDelay( interval / portTICK_RATE_MS);
    }
  }

  if (start>stop) {
    interval = interval / (start - stop);
    for(int i=start; i>=stop; i-=divider) {
      current_brightness = set_brightness(i);
      if (interval<10) interval=10;
      vTaskDelay( interval / portTICK_RATE_MS);
    }
  }
}

int toggle_dimmer() {
  int new_level;
  if (current_brightness > 25) {
    new_level = 0;
  } else {
    new_level = 255;
  }

  lwsl_notice("toggle dimmer from %d to %d\n",current_brightness,new_level);
  current_brightness = set_brightness(new_level);
  return new_level;
}

void IRAM_ATTR timer_group1_isr(void *para) {

    int timer_idx = (int) para;

    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value =
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
        | TIMERG0.hw_timer[timer_idx].cnt_low;

    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1 && !storage_in_use) {
        gpio_set_level(TRIAC_IO, TRIAC_ON);
        if (current_brightness) {
          gpio_set_level(TRIAC_IO, TRIAC_ON);
        } else {
          gpio_set_level(TRIAC_IO, TRIAC_OFF);
        }
        TIMERG0.int_clr_timers.t1 = 1;
    }
}

static void tg1_timer_init(int timer_idx,
    bool auto_reload, double timer_interval_sec) {
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_1, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_1, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_1, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_1, timer_idx);
    timer_isr_register(TIMER_GROUP_1, timer_idx, timer_group1_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_1, timer_idx);
}

static void IRAM_ATTR zerocross_isr(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    if (current_brightness < max_brightness) gpio_set_level(TRIAC_IO, TRIAC_OFF);
    tg1_timer_init(TIMER_1, TEST_WITH_RELOAD, triac_delay);
    zerocross_present = true;
    // xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void dimmer_gpio_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

		io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(ZERO_DETECT_IO, zerocross_isr, (void*) ZERO_DETECT_IO);
}

static void dimmer_service(void *pvParameter) {
    uint32_t io_num;
    current_brightness = set_brightness(255); //start with dimmmer on
    while (1) {

      if (!dimmer_enabled) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        continue;
      }

      //incoming messages from other services
      if (dimmer_payload) {
        if (cJSON_GetObjectItem(dimmer_payload,"level")) {
          int level = cJSON_GetObjectItem(dimmer_payload,"level")->valueint;
          current_brightness = set_brightness(level);
          lwsl_notice("[dimmer_service] level %d\n",level);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"on")) {
          int power = cJSON_GetObjectItem(dimmer_payload,"on")->valueint;
          if (power == 1) {
            if (current_brightness == 0) current_brightness = max_brightness;
            current_brightness = set_brightness(current_brightness);
          } else if (power == 0) {
            // we do not set current_brightness
            // so 'on' returns to previous brightness
            set_brightness(0);
          }

          lwsl_notice("[dimmer_service] power %d\n",power);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"toggle")) {
          toggle_dimmer();
          lwsl_notice("[dimmer_service] toggle %d\n",current_brightness);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"increment")) {
          int increment = cJSON_GetObjectItem(dimmer_payload,"increment")->valueint;
          inc_brightness(increment);
          lwsl_notice("[dimmer_service] increment %d\n",increment);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"decrement")) {
          int decrement = cJSON_GetObjectItem(dimmer_payload,"decrement")->valueint;
          dec_brightness(decrement);
          lwsl_notice("[dimmer_service] decrement %d\n",decrement);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"fade")) {
          int fade = cJSON_GetObjectItem(dimmer_payload,"fade")->valueint;
          fade_brightness(0,fade,1000);
          lwsl_notice("[dimmer_service] level %d\n",fade);
        }

        if (cJSON_GetObjectItem(dimmer_payload,"neutral_present")) {
          if (cJSON_IsTrue(cJSON_GetObjectItem(dimmer_payload,"neutral_present"))) {
            NEUTRAL_PRESENT = 1;
          } else {
            NEUTRAL_PRESENT = 0;
          }
          lwsl_notice("[dimmer_service] neutral_present: %d\n",NEUTRAL_PRESENT);
        }

        dimmer_payload = NULL;
      }

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void dimmer_main() {
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));
  //xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 10, NULL);
  xTaskCreate(&dimmer_service, "dimmer_service_task", 5000, NULL, 5, NULL);
  dimmer_gpio_init();
  printf("starting dimmer service\n");
  //xTaskCreate(&dimmer_service, "dimmer_service_task", 5000, NULL, 5, NULL);
  return 0;
}
