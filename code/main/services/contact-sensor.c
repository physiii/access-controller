#include "drivers/binary_switch.c"

char contact_service_message[2000];
char contact_service_message_in[2000];
bool contact_service_message_ready = false;

static void
contact_service(void *pvParameter)
{
  while (1) {
    //outgoing messages to other services
    if (binary_switch_event()) {
        sprintf(contact_service_message,""
        "{\"event_type\":\"service/\state\","
        " \"payload\":"
        "{\"id\":\"1\","
        "\"state\":{\"contact\":%d}}}"
        , get_switch_level());
        contact_service_message_ready = true;
        printf("%s\n", contact_service_message);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

int
contact_main()
{
  binary_switch_main();
  printf("starting contact sensor service\n");
  xTaskCreate(&contact_service, "contact_service_task", 5000, NULL, 5, NULL);
  return 0;
}
