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
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

char alarm_service_message[2000];
char alarm_service_message_in[2000];
bool alarm_service_message_ready = false;
char alarm_state[1000];

bool system_armed_state = true;

void armSystem(int state) {
  if (state) {
    system_armed_state = true;
  } else {
    system_armed_state = false;
  }
}

bool isArmed() {
  return system_armed_state;
}

void store_alarm_state(int mode) {
  snprintf(alarm_state,sizeof(alarm_state),"{\"mode\":%d}",mode);
  store_char("alarm_state",alarm_state);
  printf("Storing alarm state:\t%s\n", alarm_state);
}

void load_alarm_state() {
  strcpy(alarm_state, get_char("alarm_state"));
  alarm_payload = cJSON_Parse(alarm_state);
  printf("Loading alarm state, setting payload:\t%s\n", alarm_state);
}

void createAlarmServiceMessage () {
  snprintf(alarm_service_message,sizeof(alarm_service_message),""
    "{\"event_type\":\"service/state\","
    " \"payload\":{\"service_id\":\"alarm_1\",\"state\":{\"value\":\"triggerAlarm\",\"mode\":1}}}"
    );

  alarm_service_message_ready = true;
}

static void alarm_service(void *pvParameter) {
    load_alarm_state();
    while (1) {

        //incoming messages from other services
        if (alarm_payload) {
          if (cJSON_GetObjectItem(alarm_payload,"mode")) {
            int mode = cJSON_GetObjectItem(alarm_payload,"mode")->valueint;
            armSystem(mode);
            store_alarm_state(mode);
            lwsl_notice("[alarm_service] mode %d\n",mode);
          }

          alarm_payload = NULL;
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void alarm_main() {
  //xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 10, NULL);
  xTaskCreate(&alarm_service, "alarm_service_task", 5000, NULL, 5, NULL);
  printf("starting alarm service\n");
  //xTaskCreate(&alarm_service, "alarm_service_task", 5000, NULL, 5, NULL);
  return 0;
}
