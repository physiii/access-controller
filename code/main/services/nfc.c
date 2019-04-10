#include "drivers/rc522/rc522_2.c"

cJSON *nfc_payload = NULL;
char nfc_service_message[2000];
bool nfc_service_message_ready = false;
cJSON *authorized_nfc_uids = NULL;

int store_nfc_uids(cJSON * uids) {
  printf("Storing UIDs: %s\n",cJSON_PrintUnformatted(uids));
  store_char("nfc_uids", cJSON_PrintUnformatted(uids));
  return 0;
}

int load_nfc_uids_from_flash() {
  char *nfc_uids_str = get_char("nfc_uids");
  if (strcmp(nfc_uids_str,"")==0) {
    printf("nfc_uids not found in flash.\n");
    return 1;
  }

  // Need JSON validation
  cJSON *tmp = cJSON_Parse(nfc_uids_str);
  authorized_nfc_uids = cJSON_GetObjectItemCaseSensitive(tmp,"uids");
  printf("Loaded nfc_uids from flash. %s\n", cJSON_PrintUnformatted(authorized_nfc_uids));
  return 0;
}

bool is_uid_authorized(char * uid) {
  cJSON *uid_obj =  NULL;
  char current_uid[20];

  cJSON_ArrayForEach(uid_obj, authorized_nfc_uids)
  {
    sprintf(current_uid,"%s",uid_obj->valuestring);
    if (strcmp(current_uid,uid)==0) {
      return true;
    }
  }

  return false;
}

static void nfc_service(void *pvParameter) {

  uint32_t io_num;
  rc522_main();
  load_nfc_uids_from_flash();
  uint8_t r ;

  // char str[100];
  // strcpy(str,"{\"uids\":[\"2ed34be1\",\"45f77a853280\",\"446352bcd4280\"]}");
  // store_nfc_uids(cJSON_Parse(str));

  while (1) {
    // checking if a new card was seen
    if (new_card_found()) {
      if (is_uid_authorized(get_card_uid())) {
        printf("UID (%s) authorized, access granted\n", get_card_uid());
        debounce_nfc = relock_delay;
        arm_lock(false);
        vTaskDelay(relock_delay / portTICK_RATE_MS);
        arm_lock(true);
      } else {
        printf("UID (%s) not authorized\n", get_card_uid());
      }
    }

    // incoming messages from other services
    if (nfc_payload) {
      nfc_payload = NULL;
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void nfc_main() {
  printf("starting nfc service\n");
  TaskHandle_t nfc_service_task;
  xTaskCreate(&nfc_service, "nfc_service_task", 5000, NULL, 5, NULL);
}
