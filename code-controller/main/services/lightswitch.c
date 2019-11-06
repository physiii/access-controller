#define LIGHT_1_IO CONFIG_LIGHT_1_IO
#define LIGHT_2_IO CONFIG_LIGHT_2_IO

#define SWITCH_1_IO CONFIG_SWITCH_1_IO
#define SWITCH_2_IO CONFIG_SWITCH_2_IO

#define LIGHT_OFF_TIME CONFIG_LIGHT_OFF_TIME

static xQueueHandle switch_evt_queue = NULL;

struct lightswitch
{
  int light_io;
  int switch_io;
  bool light_state;
  bool switch_state;
};

struct lightswitch ls1;
struct lightswitch ls2;

bool light_off_timer_expired = true;
int light_off_count = 0;

static void IRAM_ATTR switch_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(switch_evt_queue, &gpio_num, NULL);
}

void start_light_off_timer(bool val)
{
  if (val) {
    light_off_timer_expired = false;
    light_off_count = 0;
  } else {
    light_off_timer_expired = true;
  }
}

void light_on (struct lightswitch ls, bool val) {
  gpio_set_level(ls.light_io, val);
  ls.light_state = val;
  printf("Light %d\t state %d\n",ls.light_io,ls.light_state);
}

void lights_on (bool val) {
  light_on(ls1, val);
  light_on(ls2, val);
}

void light_motion (bool val) {
  if (val) {
    lights_on(val);
  } else {
    start_light_off_timer(true);
  }
}

static void light_off_timer(void *pvParameter)
{
  while (1) {
    if (light_off_count >= LIGHT_OFF_TIME && !light_off_timer_expired) {
      light_off_timer_expired = true;
      lights_on(false);
    } else light_off_count++;
    vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
  }
}

static void lightswitch_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(switch_evt_queue, &io_num, portMAX_DELAY)) {
            int state = gpio_get_level(io_num);
            if (io_num == ls1.switch_io) {
              light_on(ls1, state);
            }
            if (io_num == ls2.switch_io) {
              light_on(ls2, state);
            }
        }
    }
}

void lightswitch_main(void)
{
    printf("Starting lightswitch_main.\n");
    ls1.switch_io = SWITCH_1_IO;
    ls2.switch_io = SWITCH_2_IO;
    ls1.light_io = LIGHT_1_IO;
    ls2.light_io = LIGHT_2_IO;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = ((1ULL<<ls1.switch_io) | (1ULL<<ls2.switch_io));
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    switch_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(lightswitch_task, "lightswitch_task", 2048, NULL, 10, NULL);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(ls1.switch_io, switch_isr_handler, (void*) ls1.switch_io);
    gpio_isr_handler_add(ls2.switch_io, switch_isr_handler, (void*) ls2.switch_io);

    xTaskCreate(light_off_timer, "light_off_timer", 2048, NULL, 10, NULL);
}
