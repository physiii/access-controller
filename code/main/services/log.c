char log_service_message[500];
bool log_service_message_ready = false;
uint32_t log_cnt = 0;

void createLogServiceMessage (char * log)
{
  snprintf(wss_data_out,sizeof(wss_data_out),""
  "{\"event_type\":\"log\","
  " \"payload\":{\"type\":\"log\","
  " \"log\":%s}}"
  , log);

  printf("sending log...\n%s\n",wss_data_out);
  wss_data_out_ready = true;
  vTaskDelay(50 / portTICK_RATE_MS);
}

void createLogCallbackMessage (char * log_id, int callback_id)
{
  char log_str[200];

  strcpy(log_str,get_char(log_id));

  // Reply with callback
  snprintf(wss_data_out,sizeof(wss_data_out),""
  "{\"id\":%d,\"callback\":true,\"payload\":[false,[%s]]}"
  ,callback_id, log_str);
  printf("sending callback...\n%s\n",wss_data_out);
  wss_data_out_ready = true;
  vTaskDelay(50 / portTICK_RATE_MS);
}

int store_log (struct access_log * log)
{
  char log_str[200];
  char log_id[25];
  char registered[5] = "true";
  char granted[5] = "true";
  log_cnt++;
  (log->registered) ? strcpy(registered,"true") : strcpy(registered,"false");
  (log->granted) ? strcpy(granted,"true") : strcpy(granted,"false");
  sprintf(log->log_id, "log_%u", log_cnt);

  sprintf(log_str,""
  "{\"log_id\":\"%s\""
  ",\"date\":\"%s\""
  ",\"key_id\":\"%s\""
  ",\"name\":\"%s\""
  ",\"key_registered\":%s"
  ",\"access_granted\":%s}",
  log->log_id, log->date, log->key_id, log->name, registered, granted);

  store_char(log_id, log_str);
  store_u32("log_cnt", log_cnt);
  createLogServiceMessage(log_str);
  return 0;
}

int load_log_count_from_flash()
{
  log_cnt = get_u32("log_cnt", log_cnt);
  return 0;
}

int load_log_from_flash()
{
  // char *log = get_char("log");
  // if (strcmp(log,"")==0) {
  //   printf("log not found in flash.\n");
  //   return 1;
  // } else {
  //   printf("log found in flash.\n%s\n",log);
  // }
  //
  // printf("Loaded log from flash. %s\n", log);

  return 0;
}

int handle_log_action(char * action)
{
  printf("log action: %s\n",action);

	return 0;
}

static void log_service (void *pvParameter)
{
  load_log_count_from_flash();
  log_payload = cJSON_CreateObject();

  while (1)
  {
    // incoming messages from other services
    if (log_payload) {
      for (int i=0; i < log_cnt; i++) {
        char log_id[25];
        sprintf(log_id, "log_%d", i);
        createLogCallbackMessage(log_id, callback_id);
      }
      log_payload = NULL;
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void log_main() {
  printf("starting log service\n");
  TaskHandle_t log_service_task;
  xTaskCreate(&log_service, "log_service_task", 5000, NULL, 5, NULL);
}
