// #include "drivers/PYQ2898.c"
#include "drivers/AM312.c"

int MOTION_DEBOUNCE = 20 * 1000;

char motion_service_message[2000];
bool motion_service_message_ready = false;
bool debounce_motion = false;
unsigned int debounce_flag = 0;
int motion_duration = 0;
int motion_duration_threshold = 2;
int motion_threshold = 0;
int motion_relock_delay = 0;

void createMotionServiceMessage () {
  sprintf(motion_service_message,""
  "{\"event_type\":\"motion/active\","
  " \"payload\":{\"type\":\"PIR\","
  " \"duration\":%d}}"
  , motion_duration);
  printf("%s\n", motion_service_message);

  motion_service_message_ready = true;
}

void set_motion_threshold(int sensitivity) {
  motion_threshold = sensitivity;
}

int store_motion_settings(cJSON * settings) {
  store_char("motion_settings", cJSON_PrintUnformatted(settings));
  return 0;
}

int load_motion_settings_from_flash() {
  char * motion_settings_str = get_char("motion_settings");
  if (strcmp(motion_settings_str,"")==0) {
    printf("No motion settings found in flash.\n");
    return 1;
  } else {
    printf("Loading motion settings from flash. %s\n", motion_settings_str);
  }

  // Need JSON validation
  motion_payload = cJSON_Parse(motion_settings_str);

  return 0;
}

static void motion_service(void *pvParameter) {
  pir_main();
  uint32_t io_num;
  printf("motion service loop\n");
  load_motion_settings_from_flash();

  while (1) {
    if (get_motion_state()){
      motion_duration++;
      if (motion_duration > motion_duration_threshold) {
        arm_lock(false);

        createMotionServiceMessage();
        // if (isArmed()) createAlarmServiceMessage();
        createDimmerServiceMessage(BUTTON_UP);
        vTaskDelay(MOTION_DEBOUNCE / portTICK_PERIOD_MS);
      }
    } else {
      vTaskDelay(motion_relock_delay * 1000 / portTICK_RATE_MS);
      if (!isLockArmed) arm_lock(true);
      motion_duration = 0;
    }

    //incoming messages from other services
    if (motion_payload) {
      if (cJSON_GetObjectItem(motion_payload,"sensitivity")) {
        int sensitivity = cJSON_GetObjectItem(motion_payload,"sensitivity")->valueint;
        set_motion_threshold(sensitivity);
        store_motion_settings(motion_payload);
        lwsl_notice("[motion_service] sensitivity %d\n",sensitivity);
      }

      motion_payload = NULL;
    }

    // printf("Motion Level:\t%d\n",motion_level);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

int motion_main() {
  printf("starting motion service\n");
  xTaskCreate(&motion_service, "motion_service_task", 5000, NULL, 5, NULL);
  return 0;
}
