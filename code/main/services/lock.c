#define LOCK_IO    0

cJSON *lock_payload = NULL;
char lock_service_message[2000];
bool lock_service_message_ready = false;
int relock_delay = 4 * 1000;

int store_lock_state(cJSON * state) {
  printf("Storing lock state: %s\n",cJSON_PrintUnformatted(state));
  store_char("lock", cJSON_PrintUnformatted(state));
  return 0;
}

int load_lock_state_from_flash() {
  char *state_str = get_char("lock");
  if (strcmp(state_str,"")==0) {
    printf("Lock state not found in flash.\n");
    return 1;
  }

  // Need JSON validation
  cJSON *lock_payload = cJSON_Parse(state_str);
  printf("Loaded lock state from flash. %s\n", state_str);
  return 0;
}

void arm_lock (bool val) {
  if (val) {
    setLED(255, 0, 0);
    gpio_set_level(LOCK_IO, val);
    printf("Lock armed.\n");
  } else {
    setLED(0, 255, 0);
    gpio_set_level(LOCK_IO, val);
    printf("Lock disarmed.\n");
  }
  isLockArmed = val;
}

void lock_init() {
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = LOCK_IO;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

static void lock_service(void *pvParameter) {
  load_lock_state_from_flash();

  while (1) {
    // incoming messages from other services
    if (lock_payload) {

      if (cJSON_GetObjectItem(lock_payload,"arm")) {
        bool arm = cJSON_IsTrue(cJSON_GetObjectItem(lock_payload,"arm"));
        arm_lock(arm);
        lwsl_notice("[lock_service] arm %d\n",arm);
      }

      lock_payload = NULL;
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void lock_main() {
  printf("starting lock service\n");
  TaskHandle_t lock_service_task;
  xTaskCreate(&lock_service, "lock_service_task", 5000, NULL, 5, NULL);
}
