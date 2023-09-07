#define MAX_UIDS			400
#define MAX_UID_SIZE	50

int CHECK_UID = 0;
int ADD_UID = 1;
int REMOVE_UID = 2;

char uids[MAX_UIDS][MAX_UID_SIZE];
char uids_tmp[MAX_UIDS][MAX_UID_SIZE];

int current_mode = 0;
char auth_service_message[1000];
bool auth_service_message_ready = false;
cJSON *auth_uids = NULL;
cJSON *new_auth_uids = NULL;

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
  cJSON *id = NULL;
  char current_id[50] = { 0 };

	printf("add_auth_uid %s\n",new_id);
  cJSON_ArrayForEach(id, auth_uids) {
    if (cJSON_IsString(id)) {
      strcpy(current_id,id->valuestring);
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
  cJSON *id =  NULL;
	int id_cnt = 0;
  char current_id[50];
	new_auth_uids = cJSON_CreateArray();

	cJSON_ArrayForEach(id, auth_uids) {
    strcpy(current_id,id->valuestring);
		if (strcmp(current_id, target_id)==0) {
			// printf("Found match for remove target %s...%s\n", target_id, current_id);
		} else {
      // printf("Adding UID %s.\n",current_id);
			cJSON *id_obj =  cJSON_CreateString(current_id);
			sprintf(uids[id_cnt], "%s", current_id);
			id_cnt++;
		}
	}

	for (int i = 0; i < id_cnt; i++) {
		cJSON *id_obj =  cJSON_CreateString(uids[i]);
		cJSON_AddItemToArray(new_auth_uids, id_obj);
	}

	auth_uids = new_auth_uids;
  store_auth_uids(auth_uids);
}

bool is_uid_authorized (char * uid)
{
  cJSON *uid_obj =  NULL;
  char current_uid[20] = { 0 };

  cJSON_ArrayForEach(uid_obj, auth_uids) {
    sprintf(current_uid,"%s",uid_obj->valuestring);

    if (strcmp(current_uid,uid)==0) {
			printf("UID Authorized.\n");
      return true;
    }
  }

	printf("UID is NOT Authorized.\n");
  return false;
}

int handle_uid (char * uid)
{

  if (current_mode == ADD_UID) {
    add_auth_uid(uid);
    return 0;
  }

  if (current_mode == REMOVE_UID) {
    remove_auth_uid(uid);
    return 0;
  }

  if (current_mode == CHECK_UID) {
		return is_uid_authorized(uid);
  }

  printf("Access granted to %s.\n", uid);
  return 0;
}

void sendUsers()
{
	char users_str[1000] = "";
	sprintf(users_str,
		"{\"eventType\":\"users\", "
		"\"payload\":%s}",
		cJSON_PrintUnformatted(auth_uids));

	printf("users: %s\n", users_str);
	addClientMessageToQueue(users_str);
}

void handle_users_message(cJSON * payload)
{
	char user[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendUsers();
	}

	if (cJSON_GetObjectItem(payload,"addUser")) {
		 int addUser = cJSON_GetObjectItem(payload,"addUser")->valueint;
		 if (addUser) {
			 current_mode = ADD_UID;
		 } else {
			 current_mode = CHECK_UID;
		 }
	}

	if (cJSON_GetObjectItem(payload,"removeUser")) {
		 char user[100] = "";
		 sprintf(user, "%s", cJSON_GetObjectItem(payload,"removeUser")->valuestring);
		 remove_auth_uid(user);
	}
}

static void auth_service (void *pvParameter)
{
  uint32_t io_num;
  uint8_t r;
  load_auth_uids_from_flash();

  while (1) {
		handle_users_message(checkServiceMessage("users"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void auth_main()
{
  printf("starting auth service\n");
  TaskHandle_t auth_service_task;
  xTaskCreate(&auth_service, "auth_service_task", 5000, NULL, 5, NULL);
}
