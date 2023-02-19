char auth_service_message[1000];
bool auth_service_message_ready = false;
cJSON *auth_uids = NULL;

int store_auth_uids(cJSON * uids)
{
  printf("Storing UIDs: %s\n",cJSON_PrintUnformatted(uids));
  store_char("auth_uids", cJSON_PrintUnformatted(uids));
  return 0;
}

int load_auth_uids_from_flash()
{
  char *uids = get_char("auth_uids");
  if (strcmp(uids,"")==0) {
    printf("auth_uids not found in flash.\n");
    auth_uids = cJSON_CreateArray();
    return 1;
  } else {
    printf("auth_uids found in flash.\n%s\n",uids);
  }

  cJSON *obj = cJSON_Parse(uids);
  if (cJSON_IsArray(obj)) {
    auth_uids = obj;
    printf("Loaded auth_uids from flash. %s\n", uids);
  } else {
    printf("auth_uids are not in a json array\n");
  }

  return 0;
}

void add_auth_uid (char * new_id)
{
  cJSON *id =  NULL;
  char current_id[50];

  cJSON_ArrayForEach(id, auth_uids) {
    if (cJSON_IsString(id)) {
      strcpy(current_id,id->valuestring);
      printf("Current UID is %s.\n",current_id);
      if (strcmp(current_id, new_id)==0) {
        printf("UID Already added.\n");
        return;
      }
    }
  }
  cJSON *id_obj =  cJSON_CreateString(new_id);
  cJSON_AddItemToArray(auth_uids, id_obj);
  store_auth_uids(auth_uids);
}

void remove_auth_uid (char * target_id)
{
  cJSON *new_auth_uids = cJSON_CreateArray();
  cJSON *id =  NULL;
  char current_id[50];

  cJSON_ArrayForEach(id, auth_uids) {
    // if (cJSON_IsString(id)) {
      strcpy(current_id,id->valuestring);
      // printf("Current UID is %s.\n",current_id);
      if (strcmp(current_id, target_id)==0) {
        printf("Found match for target %s...%s\n", target_id, current_id);
      } else {
        cJSON_AddItemToArray(new_auth_uids,id);
        printf("Add UID %s.\n",current_id);
      }
    // }
  }

  store_auth_uids(new_auth_uids);
}

bool is_uid_authorized (char * uid)
{
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

static void auth_service (void *pvParameter)
{
  uint32_t io_num;
  uint8_t r ;
  load_auth_uids_from_flash();

  while (1) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void auth_main()
{
  printf("starting auth service\n");
  TaskHandle_t auth_service_task;
  xTaskCreate(&auth_service, "auth_service_task", 5000, NULL, 5, NULL);
}
