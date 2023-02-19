#define PIR_IO CONFIG_PIR_IO

#define GPIO_INPUT_PIN_SEL  ((1ULL<<PIR_IO))

static xQueueHandle motion_evt_queue = NULL;

bool motion_state = false;
bool previous_motion_state = false;
bool new_motion_event_flag = false;

static void IRAM_ATTR motion_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(motion_evt_queue, &gpio_num, NULL);
}

int new_motion_event() {
  bool flag = new_motion_event_flag;
  new_motion_event_flag = false;
  return flag;
}

int get_motion_state() {
  return motion_state;
}

static void motion_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(motion_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == PIR_IO) {
              motion_state = gpio_get_level(io_num);
              if (previous_motion_state != motion_state) {
                new_motion_event_flag = true;
              }
              previous_motion_state = motion_state;
            }
        }
    }
}

void motion_main(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    motion_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(motion_task, "motion_task", 2048, NULL, 10, NULL);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIR_IO, motion_isr_handler, (void*) PIR_IO);
}
