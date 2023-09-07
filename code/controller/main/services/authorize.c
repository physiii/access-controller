#define MAX_UIDS			400
#define MAX_UID_SIZE	50

#define MAX_USERS		400
#define MAX_UID_SIZE	50
#define MAX_NAME_SIZE   50
#define MAX_PIN_SIZE    10

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

typedef struct {
    char uid[MAX_UID_SIZE];
    char name[MAX_NAME_SIZE];
    char pin[MAX_PIN_SIZE];
} user_info;

user_info users[MAX_USERS];
cJSON *auth_users = NULL;

void store_users_to_flash() {
    auth_users = cJSON_CreateArray();
    for (int i = 0; i < MAX_USERS; i++) {
      if (strlen(users[i].name) == 0) continue;
      cJSON *user = cJSON_CreateObject();
      cJSON_AddStringToObject(user, "name", users[i].name);
      cJSON_AddStringToObject(user, "pin", users[i].pin);
      cJSON_AddItemToArray(auth_users, user);
    }

    char *auth_users_str = cJSON_PrintUnformatted(auth_users);  // Allocated dynamically
    if (auth_users_str != NULL) {
        printf("Storing users to flash: %s\n", auth_users_str);
        store_char("auth_users", auth_users_str);
        free(auth_users_str);  // Free the allocated memory
    } else {
        ESP_LOGE(TAG, "Failed to print JSON");
    }
}

void load_users_from_flash() {
    char *user_str = get_char("auth_users");
    if (user_str == NULL || strcmp(user_str, "") == 0) {
        printf("No users found in flash.\n");
        return;
    }
    cJSON *loaded_users = cJSON_Parse(user_str);
    if (loaded_users == NULL || !cJSON_IsArray(loaded_users)) {
        printf("Corrupt user data in flash.\n");
        return;
    }

    int i = 0;
    cJSON *user = NULL;
    cJSON_ArrayForEach(user, loaded_users) {
        cJSON *name = cJSON_GetObjectItem(user, "name");
        cJSON *pin = cJSON_GetObjectItem(user, "pin");
        if (name && pin) {
            strncpy(users[i].name, name->valuestring, MAX_NAME_SIZE);
            strncpy(users[i].pin, pin->valuestring, MAX_PIN_SIZE);
            i++;
        }
        printf("Loaded user: Name = %s, Pin = %s\n", name->valuestring, pin->valuestring);
    }
    cJSON_Delete(loaded_users);
    free(user_str);
}

void sendUsers() {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strlen(users[i].name) == 0) continue;

        cJSON *user = cJSON_CreateObject();
        cJSON_AddStringToObject(user, "name", users[i].name);
        cJSON_AddStringToObject(user, "pin", users[i].pin);

        // Create the message JSON array with only this user
        cJSON *users_json = cJSON_CreateArray();
        cJSON_AddItemToArray(users_json, user);

        char *users_json_str = cJSON_PrintUnformatted(users_json);  // This allocates memory
        if (users_json_str == NULL) {
            // Handle error
            ESP_LOGE(TAG, "Failed to print JSON");
            cJSON_Delete(users_json);
            continue;
        }

        char msg[2000] = "";
        snprintf(msg, sizeof(msg), "{\"event_type\":\"load\", \"payload\":{\"services\":[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":{\"users\":%s}}]}}", users_json_str);

        free(users_json_str);  // Free the allocated memory

        addServerMessageToQueue(msg);

        cJSON_Delete(users_json);
    }
    // addClientMessageToQueue(users_str);
}

void addUser(char *name, char *pin) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].name, name) == 0) {
            printf("User with name %s already exists.\n", name);
            return;
        }
    }
    printf("Adding user 1: Name = %s, Pin = %s\n", name, pin);
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].name, "") == 0) {
          strncpy(users[i].name, name, MAX_NAME_SIZE);
          strncpy(users[i].pin, pin, MAX_PIN_SIZE);
          store_users_to_flash();
          printf("User added: Name = %s, Pin = %s\n", name, pin);
          sendUsers();
          return;
        }
    }
    printf("User limit reached. Cannot add more users.\n");
}

void modifyUser(char *name, char *newName, char *newPin) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].name, name) == 0) {
            strncpy(users[i].name, newName, MAX_NAME_SIZE);
            strncpy(users[i].pin, newPin, MAX_PIN_SIZE);
            store_users_to_flash();
            printf("User modified: New Name = %s, New Pin = %s\n", newName, newPin);
            sendUsers();
            return;
        }
    }
    printf("No user with name %s found to modify.\n", name);
}

void removeUser(char *name) {
    printf("Removing user with name %s\n", name);
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].name, name) == 0) {
            memset(&users[i], 0, sizeof(user_info));
            store_users_to_flash();
            printf("User with name %s removed.\n", name);
            sendUsers();
            return;
        }
    }
    printf("No user with name %s found.\n", name);
}

int is_pin_authorized(const char *incomingPin) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strlen(users[i].name) == 0) continue;  // Skip uninitialized users
        if (strcmp(users[i].pin, incomingPin) == 0) {
            return 1;  // PIN found and authorized
        }
    }
    return 0;  // PIN not found, not authorized
}

void handle_authorize_message(cJSON * payload)
{ 
	char user[250];
  char name[100] = "";
  char pin[10] = "";
	char property[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendUsers();
	}

  if (cJSON_GetObjectItem(payload,"uuid")) {
    sprintf(device_id, "%s", cJSON_GetObjectItem(payload,"uuid")->valuestring);
    printf("device ID received: %s\n", device_id); 
    store_char("device_id", device_id);
    cJSON_free(payload);
	}

  if (cJSON_GetObjectItem(payload,"token")) {
    sprintf(token, "%s", cJSON_GetObjectItem(payload,"token")->valuestring);
    printf("token received: %s\n", token);
    store_char("token",token);
    cJSON_free(payload);
	}

    if (cJSON_GetObjectItem(payload,"property")) {
        sprintf(property, "%s", cJSON_GetObjectItem(payload,"property")->valuestring);

        if (strcmp(property, "addUser") == 0) {
            printf("handle_authorize_message: %s\n",cJSON_PrintUnformatted(payload));
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            char name[MAX_NAME_SIZE] = "";
            char pin[MAX_PIN_SIZE] = "";
            if (cJSON_GetObjectItem(value, "name") && cJSON_GetObjectItem(value, "pin")) {
              sprintf(name, "%s", cJSON_GetObjectItem(value, "name")->valuestring);
              sprintf(pin, "%s", cJSON_GetObjectItem(value, "pin")->valuestring);
            }
            addUser(name, pin);
            cJSON_free(payload);
        }

        if (strcmp(property, "modifyUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            char name[MAX_NAME_SIZE] = "";
            char newName[MAX_NAME_SIZE] = "";
            char newPin[MAX_PIN_SIZE] = "";
            if (cJSON_GetObjectItem(value, "name") && 
                cJSON_GetObjectItem(value, "newName") && 
                cJSON_GetObjectItem(value, "newPin")) {
              sprintf(name, "%s", cJSON_GetObjectItem(value, "name")->valuestring);
              sprintf(newName, "%s", cJSON_GetObjectItem(value, "newName")->valuestring);
              sprintf(newPin, "%s", cJSON_GetObjectItem(value, "newPin")->valuestring);
            }
            modifyUser(name, newName, newPin);
            cJSON_free(payload);
        }

        printf("property: %s\n", property);
        if (strcmp(property, "removeUser") == 0) {
          cJSON *value = cJSON_GetObjectItem(payload, "value");
          printf("remove user: %s\n",cJSON_GetObjectItem(value,"name")->valuestring);
          sprintf(name, "%s", cJSON_GetObjectItem(value,"name")->valuestring);
          removeUser(name);
          cJSON_free(payload);
        }
    }

  if (cJSON_GetObjectItem(payload,"removeUser")) {
		 char user[100] = "";
		 sprintf(user, "%s", cJSON_GetObjectItem(payload,"removeUser")->valuestring);
     removeUser(user);
	}
}

static void auth_service (void *pvParameter)
{
  uint32_t io_num;
  uint8_t r;

  while (1) {
    handle_authorize_message(checkServiceMessageByAction("ac_1", "addUser"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "modifyUser"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "removeUser"));
		handle_authorize_message(checkServiceMessageByKey("uuid"));
		handle_authorize_message(checkServiceMessageByKey("token"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void auth_main()
{
  printf("starting auth service\n");
  load_users_from_flash();
  TaskHandle_t auth_service_task;
  xTaskCreate(&auth_service, "auth_service_task", 9 * 1000, NULL, 5, NULL);
}
