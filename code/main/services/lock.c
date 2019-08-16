#define LOCK_IO 18
#define LOCK_IO_BITMASK    (1ULL<<LOCK_IO)
#define FOB_IO     4
#define EXIT_BUTTON_IO     17
#define GPIO_INPUT_PIN_SEL  ((1ULL<<FOB_IO) | (1ULL<<EXIT_BUTTON_IO))
#define ESP_INTR_FLAG_DEFAULT 0

char lock_service_message[2000];
bool lock_service_message_ready = false;
int relock_delay = 4 * 1000;
int should_pulse_lock = false;

void createLockServiceMessage(bool value) {
  char value_str[10];
  if (value) {
    strcpy(value_str,"true");
  } else {
    strcpy(value_str,"false");
  }

  snprintf(lock_service_message,sizeof(lock_service_message),""
  "{\"event_type\":\"service/state\","
  " \"payload\":{\"service_id\":\"lock_1\",\"state\":{\"locked\":%s}}}"
  , value_str);
  lock_service_message_ready = true;
}

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
    // setLED(255, 0, 0);
    gpio_set_level(LOCK_IO, val);
    printf("Lock armed.\n");
  } else {
    // setLED(0, 255, 0);
    gpio_set_level(LOCK_IO, val);
    printf("Lock disarmed.\n");
  }
  isLockArmed = val;
}

void pulse_lock () {
  should_pulse_lock = true;
}

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == FOB_IO) {
              pulse_lock();
              printf("FOB: %d\n", gpio_get_level(io_num));
            }

            if (io_num == EXIT_BUTTON_IO) {
              pulse_lock();
              printf("EXIT BUTTON: %d\n", gpio_get_level(io_num));
            }

        }
    }
}

void lock_init() {
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = LOCK_IO_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_set_level(LOCK_IO, 1);

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(FOB_IO, gpio_isr_handler, (void*) FOB_IO);
    gpio_isr_handler_add(EXIT_BUTTON_IO, gpio_isr_handler, (void*) EXIT_BUTTON_IO);
}

int handle_property(char * prop) {
  printf("lock property: %s\n",prop);

	if (strcmp(prop,"lock")==0) {
    arm_lock(cJSON_IsTrue(cJSON_GetObjectItem(lock_payload,"value")));
	}

	return 0;
}

static void lock_service(void *pvParameter) {
  // load_lock_state_from_flash();

  int cnt = 0;
  while (1) {
    // incoming messages from other services
    if (should_pulse_lock) {
      arm_lock(false);
      vTaskDelay(relock_delay / portTICK_PERIOD_MS);
      arm_lock(true);
      should_pulse_lock = false;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void lock_main() {
  printf("starting lock service\n");
  lock_init();
  TaskHandle_t lock_service_task;
  xTaskCreate(&lock_service, "lock_service_task", 5000, NULL, 5, NULL);
}
