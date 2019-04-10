#include "drivers/SPH0645LM4H.c"

int microphone_threshold = 1000;
int max_microphone_level = 2000;
int microphone_scale = 255;
char microphone_service_message[2000];
bool microphone_service_message_ready = false;
bool activate_threshold = false;

void set_microphone_threshold(int value) {
  microphone_threshold = max_microphone_level * value / microphone_scale;
}

int store_microphone_settings(cJSON * settings) {
  store_char("mic_settings", cJSON_PrintUnformatted(settings));
  return 0;
}

int load_microphone_settings_from_flash() {
  char * microphone_settings_str = get_char("mic_settings");
  if (strcmp(microphone_settings_str,"")==0) {
    printf("No microphone settings found in flash.\n");
    return 1;
  } else {
    printf("Loading microphone settings from flash. %s\n", microphone_settings_str);
  }

  // Need JSON validation
  microphone_payload = cJSON_Parse(microphone_settings_str);

  return 0;
}

static void microphone_service(void *pvParameter)
{
  uint32_t io_num;
  SPH0645LM4H_main();
  load_microphone_settings_from_flash();

  while (1) {
    int level = get_microphone_level();

    // if (level < mic_threshold && !activate_threshold) {
    //   printf("mic activated\n");
    //   activate_threshold = true;
    // }

    if (level > microphone_threshold) {
      //create service message
      sprintf(microphone_service_message,""
      "{\"event_type\":\"mic/active\","
      " \"payload\":{\"type\":\"sound\","
      "\"level\":%d}}"
      , level);
      printf("%s\n", microphone_service_message);
      microphone_service_message_ready = true;

      //turn on dimmer
      cJSON *json = NULL;
      dimmer_payload = cJSON_CreateObject();
      json = cJSON_CreateNumber(1);
      cJSON_AddItemToObject(dimmer_payload, "on", json);
      
      if (isArmed()) createAlarmServiceMessage();

    }

    //incoming messages from other services
    if (microphone_payload) {
      if (cJSON_GetObjectItem(microphone_payload,"mode")) {
        int mode = cJSON_GetObjectItem(microphone_payload,"mode")->valueint;
        armSystem(mode);
        store_alarm_state(mode);
        lwsl_notice("[microphone_service] mode %d\n", mode);
      }

      if (cJSON_GetObjectItem(microphone_payload,"sensitivity")) {
        int sensitivity = cJSON_GetObjectItem(microphone_payload,"sensitivity")->valueint;
        set_microphone_threshold(sensitivity);
        store_microphone_settings(microphone_payload);
        lwsl_notice("[microphone_service] sensitivity %d\n", sensitivity);
      }

      microphone_payload = NULL;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int microphone_main() {
  printf("starting microphone service\n");
  xTaskCreate(&microphone_service, "microphone_service_task", 5000, NULL, 5, NULL);
  return 0;
}
