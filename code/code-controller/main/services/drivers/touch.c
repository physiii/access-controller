/* Touch Pad Interrupt Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/touch_pad.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

static const char* TAG = "Touch pad";
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_THRESH_PERCENT  (10)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#define PAD_NUMBER 10
bool tp_debounce = false;

static bool s_pad_activated[TOUCH_PAD_MAX];
static uint32_t s_pad_init_val[TOUCH_PAD_MAX];

#define UP_PAD 7
#define DOWN_PAD 5
#define LEFT_PAD 9
#define RIGHT_PAD 8

int touch_sensitivity = 254; //silicon button: 994
int max_touch_level = 1000;
int touch_scale = 255;
int touch_bias = 900;
bool enable_diagnols = false;
bool enable_directions = false;
int touch_threshold[PAD_NUMBER];

uint16_t touch_value;
uint16_t touch_filter_value;

static void tp_example_touch_pad_init()
{
  touch_pad_config(UP_PAD, TOUCH_THRESH_NO_USE);
  touch_pad_config(DOWN_PAD, TOUCH_THRESH_NO_USE);
  touch_pad_config(LEFT_PAD, TOUCH_THRESH_NO_USE);
  touch_pad_config(RIGHT_PAD, TOUCH_THRESH_NO_USE);
}

void tp_set_thresholds(int pad){
  //read filtered value
  uint16_t touch_value;
  touch_pad_read_filtered(pad, &touch_value);
  s_pad_init_val[pad] = touch_value;
  //set interrupt threshold.
  int val = touch_bias + (max_touch_level - touch_bias) * touch_sensitivity / touch_scale;
  ESP_ERROR_CHECK(touch_pad_set_thresh(pad, val * touch_value / max_touch_level));
  printf("[%s] tp_set_thresholds [%d] val is %d\n", TAG, pad, val);
}

void set_touch_threshold(int value) {
  touch_sensitivity = value;
  tp_set_thresholds(UP_PAD);
  tp_set_thresholds(DOWN_PAD);
  tp_set_thresholds(LEFT_PAD);
  tp_set_thresholds(RIGHT_PAD);
}

static void tp_init(int pad){
  touch_pad_config(pad, TOUCH_THRESH_NO_USE);
  tp_set_thresholds(pad);
}

bool get_pad_state(int pad) {
  bool state = s_pad_activated[pad];
  s_pad_activated[pad] = false;
  return state;
}

// disabled diaganol direction
int get_dpad_state() {

  int UP = get_pad_state(UP_PAD);
  int DOWN = get_pad_state(DOWN_PAD);
  int LEFT = get_pad_state(LEFT_PAD);
  int RIGHT = get_pad_state(RIGHT_PAD);

  if (!enable_directions) {
    if (UP || DOWN || LEFT || RIGHT) return 1;
  }

  if (enable_diagnols) {
    if (UP && RIGHT) return 2;
    if (RIGHT && DOWN) return 3;
    if (DOWN && LEFT) return 4;
    if (LEFT && UP) return 5;
  }

  if (UP && DOWN && LEFT && RIGHT) return 1;
  if (UP) return 6;
  if (RIGHT) return 7;
  if (DOWN) return 8;
  if (LEFT) return 9;

  return 0;
}

static void tp_example_rtc_intr(void * arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    //clear interrupt
    touch_pad_clear_status();
    if (tp_debounce) return;
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        if ((pad_intr >> i) & 0x01) {
            s_pad_activated[i] = true;
            tp_debounce = true;
        }
    }

}

void touch_main()
{
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing touch pad");
    touch_pad_init();
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Set reference voltage for charging/discharging
    // For most usage scenarios, we recommend using the following combination:
    // the high reference valtage will be 2.7V - 1V = 1.7V, The low reference voltage will be 0.5V.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    // Init touch pad IO
    //tp_example_touch_pad_init();

    touch_pad_config(UP_PAD, TOUCH_THRESH_NO_USE);
    touch_pad_config(DOWN_PAD, TOUCH_THRESH_NO_USE);
    touch_pad_config(LEFT_PAD, TOUCH_THRESH_NO_USE);
    touch_pad_config(RIGHT_PAD, TOUCH_THRESH_NO_USE);

    // Initialize and start a software filter to detect slight change of capacitance.
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // Set threshold
    tp_set_thresholds(UP_PAD);
    tp_set_thresholds(DOWN_PAD);
    tp_set_thresholds(LEFT_PAD);
    tp_set_thresholds(RIGHT_PAD);

    // Register touch interrupt ISR
    touch_pad_isr_register(tp_example_rtc_intr, NULL);
    // Start a task to show what pads have been touched
    touch_pad_intr_enable();
    //xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
