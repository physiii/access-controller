#include "drivers/touch.c"

#define BUTTON_RELEASE 0
#define BUTTON_CENTER 1
#define BUTTON_UP 6
#define BUTTON_RIGHT 7
#define BUTTON_DOWN 8
#define BUTTON_LEFT 9
#define BUTTON_UP_RIGHT 2
#define BUTTON_RIGHT_DOWN 3
#define BUTTON_DOWN_LEFT 4
#define BUTTON_LEFT_UP 5

char buttons_service_message[2000];
bool buttons_service_message_ready = false;
char button_direction[100];
int button_debounce_time = 500; // debounce time in ms

int store_button_settings(cJSON * settings) {
  store_char("button_settings", cJSON_PrintUnformatted(settings));
  return 0;
}

int load_button_settings_from_flash() {
  char * button_settings_str = get_char("button_settings");
  if (strcmp(button_settings_str,"")==0) {
    printf("No button settings found in flash.\n");
    return 1;
  } else {
    printf("Loading button settings from flash. %s\n", button_settings_str);
  }

  // Need JSON validation
  button_payload = cJSON_Parse(button_settings_str);

  return 0;
}

void createButtonServiceMessage(int state) {
  cJSON *level_json = NULL;
  int level;

  switch(state) {
    case BUTTON_CENTER:
      strcpy(button_direction,"center");
      break;

    case BUTTON_UP:
      strcpy(button_direction,"up");
      break;

    case BUTTON_RIGHT:
      strcpy(button_direction,"right");
      break;

    case BUTTON_DOWN:
      strcpy(button_direction,"down");
      break;

    case BUTTON_LEFT:
      strcpy(button_direction,"left");
      break;

    case BUTTON_UP_RIGHT:
      break;

    case BUTTON_RIGHT_DOWN:
      break;

    case BUTTON_DOWN_LEFT:
      break;

    case BUTTON_LEFT_UP:
      break;

    case BUTTON_RELEASE:
      break;

    default:
      break;
  }

  snprintf(buttons_service_message,sizeof(buttons_service_message),""
  "{\"event_type\":\"service/state\","
  " \"payload\":{\"service_id\":\"button_1\",\"state\":{\"value\":\"%s\"}}}"
  , button_direction);
  buttons_service_message_ready = true;
}

void createDimmerServiceMessage(int state) {
  cJSON *level_json = NULL;
  int level;

  switch(state) {
    case BUTTON_CENTER:
      dimmer_payload = cJSON_CreateObject();
      level_json = cJSON_CreateBool(true);
      cJSON_AddItemToObject(dimmer_payload, "toggle", level_json);
      strcpy(button_direction,"center");
      break;

    case BUTTON_UP:
      dimmer_payload = cJSON_CreateObject();
      level_json = cJSON_CreateNumber(255);
      cJSON_AddItemToObject(dimmer_payload, "level", level_json);
      strcpy(button_direction,"up");
      break;

    case BUTTON_RIGHT:
      dimmer_payload = cJSON_CreateObject();
      level_json = cJSON_CreateNumber(30);
      cJSON_AddItemToObject(dimmer_payload, "increment", level_json);
      strcpy(button_direction,"right");
      break;

    case BUTTON_DOWN:
      dimmer_payload = cJSON_CreateObject();
      level_json = cJSON_CreateNumber(0);
      cJSON_AddItemToObject(dimmer_payload, "level", level_json);
      strcpy(button_direction,"down");
      break;

    case BUTTON_LEFT:
      dimmer_payload = cJSON_CreateObject();
      level_json = cJSON_CreateNumber(30);
      cJSON_AddItemToObject(dimmer_payload, "decrement", level_json);
      strcpy(button_direction,"left");
      break;

    case BUTTON_UP_RIGHT:
      break;

    case BUTTON_RIGHT_DOWN:
      break;

    case BUTTON_DOWN_LEFT:
      break;

    case BUTTON_LEFT_UP:
      break;

    case BUTTON_RELEASE:
      break;

    default:
      break;
  }
}

static void
button_service(void *pvParameter)
{
  touch_main();
  load_button_settings_from_flash();

  while (1) {
      int state = get_dpad_state();

      if (state) {
        if (dimmer_enabled) createDimmerServiceMessage(state);
        createButtonServiceMessage(state);
        tp_debounce = true;
        vTaskDelay(button_debounce_time / portTICK_PERIOD_MS); //debounce
        tp_debounce = false;
      }

      //incoming messages from other services
      if (button_payload) {
        if (cJSON_GetObjectItem(button_payload,"sensitivity")) {
          int sensitivity = cJSON_GetObjectItem(button_payload,"sensitivity")->valueint;
          set_touch_threshold(sensitivity);
          store_button_settings(button_payload);
          lwsl_notice("[button_service] sensitivity %d\n",sensitivity);
        }
        button_payload = NULL;
      }

      vTaskDelay(250 / portTICK_PERIOD_MS); //debounce
  }

}

int
buttons_main() {
  xTaskCreate(&button_service, "button_service_task", 5000, NULL, 5, NULL);
  return 0;
}
