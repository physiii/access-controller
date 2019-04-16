#include "drivers/rc522/rc522_2.c"

char nfc_service_message[1000];
bool nfc_service_message_ready = false;
cJSON *auth_uids = NULL;

int store_nfc_uids(cJSON * uids) {
  printf("Storing UIDs: %s\n",cJSON_PrintUnformatted(uids));
  store_char("nfc_uids", cJSON_PrintUnformatted(uids));
  return 0;
}

int load_nfc_uids_from_flash() {
  char *uids = get_char("nfc_uids");
  if (strcmp(uids,"")==0) {
    printf("nfc_uids not found in flash.\n");
    auth_uids = cJSON_CreateArray();
    return 1;
  } else {
    printf("nfc_uids found in flash.\n%s\n",uids);
  }

  cJSON *obj = cJSON_Parse(uids);
  if (cJSON_IsArray(obj)) {
    auth_uids = obj;
    printf("Loaded nfc_uids from flash. %s\n", uids);
  } else {
    printf("nfc_uids are not in a json array\n");
  }

  return 0;
}

void add_auth_uid (char * id) {
  cJSON *id_obj =  cJSON_CreateString(id);
  cJSON_AddItemToArray(auth_uids, id_obj);
  store_nfc_uids(auth_uids);
}

void remove_auth_uid (char * target_id) {
  cJSON *new_auth_uids = cJSON_CreateArray();
  cJSON *id =  NULL;
  char current_id[50];

  cJSON_ArrayForEach(id, auth_uids) {
    if (cJSON_IsString(id)) {
      strcpy(current_id,id->valuestring);
      printf("Current UID is %s.\n",current_id);
      if (strcmp(current_id, target_id)!=0) {
        cJSON_AddItemToArray(new_auth_uids,id);
      }
    }
  }

  store_nfc_uids(new_auth_uids);
}

bool is_uid_authorized (char * uid) {
  cJSON *uid_obj =  NULL;
  char current_uid[20];

  cJSON_ArrayForEach(uid_obj, auth_uids) {
    sprintf(current_uid,"%s",uid_obj->valuestring);
    if (strcmp(current_uid,uid)==0) {
      return true;
    }
  }

  return false;
}

int handle_nfc_action(char * action) {
  printf("nfc action: %s\n",action);

  char id[50];
  if (cJSON_GetObjectItem(nfc_payload,"uid")->valuestring) {
    strcpy(id,cJSON_GetObjectItem(nfc_payload,"uid")->valuestring);
  }

	if (strcmp(action,"add")==0) {
    printf("adding: %s\n",id);
    add_auth_uid(id);
    return 1;
	}

	if (strcmp(action,"remove")==0) {
    printf("removing: %s\n",id);
		remove_auth_uid(id);
    return 1;
	}

	return 0;
}

void handle_new_card ()
{
  bool granted = false;
  bool registered = false;
  debounce_nfc = relock_delay;

  if (is_uid_authorized(get_card_uid())) {
    granted = true;
    registered = true;
    printf("UID (%s) authorized, access granted\n", get_card_uid());
  }

  struct access_log log;
  strcpy(log.key_id,get_card_uid());
  strcpy(log.name,"Joe Shmoe");
  log.registered = registered;
  log.granted = granted;
  store_log(&log);

  if (granted) {
    arm_lock(false);
    vTaskDelay(relock_delay / portTICK_RATE_MS);
    arm_lock(true);
  } else {
    printf("UID (%s) not authorized\n", get_card_uid());
    setLED(255, 0, 0);
    vTaskDelay(50 / portTICK_RATE_MS);
    setLED(0, 0, 0);
    vTaskDelay(50 / portTICK_RATE_MS);
    setLED(255, 0, 0);
    vTaskDelay(50 / portTICK_RATE_MS);
    setLED(0, 0, 0);
    vTaskDelay(50 / portTICK_RATE_MS);
    setLED(255, 0, 0);

    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

static void nfc_service (void *pvParameter)
{

  uint32_t io_num;
  rc522_main();
  load_nfc_uids_from_flash();
  uint8_t r ;

  // add_auth_uid("95eaa63");
  // add_auth_uid("446352bcd4280");

  char test_val[1000];
  char test_key[1000];
  for (int i=0; i < 25; i++) {
    sprintf(test_key,"TEST_KEY_%d",i);
    // sprintf(test_val,"{\"log_id\":\"log_19\",\"date\":\"1970-01-01T00:00:00Z\",\"key_id\":\"90fed9a4\",\"name\":\"Joe Shmoe\",\"key_registered\":false,\"access_granted\":false}");
    sprintf(test_val,"log_19 date 1970-01-01T00:00:00Z 90fed9a4 Joe Shmoe 0 0");
    store_char(test_key, test_val);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  while (1) {
    // checking if a new card was seen
    if (new_card_found()) {
      handle_new_card();
    }

    // incoming messages from other services
    if (nfc_payload) {

      if (cJSON_GetObjectItem(nfc_payload,"action")) {
        char action[50];
        sprintf(action,"%s",cJSON_GetObjectItem(nfc_payload,"action")->valuestring);
        handle_nfc_action(action);
      }

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
