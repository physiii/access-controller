#include "drivers/keypad.c"

char keypad_service_message[1000];
bool keypad_service_message_ready = false;

static void keypad_service (void *pvParameter)
{
  uint32_t io_num;
  keypad_driver_main();
  uint8_t r ;

  while (1) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void keypad_main()
{
  printf("starting keypad service\n");
  TaskHandle_t keypad_service_task;
  xTaskCreate(&keypad_service, "keypad_service_task", 5000, NULL, 5, NULL);
}
