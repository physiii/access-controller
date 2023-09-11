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

void store_user_to_flash(char *uuid, char *name, char *pin) {
    uint32_t user_count = get_u32("auth_user_count", 0);

    char key[20];
    snprintf(key, sizeof(key), "auth_user_%lu", (unsigned long)(user_count + 1));

    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "uuid", uuid);
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "pin", pin);

    char *auth_user_str = cJSON_PrintUnformatted(user);
    if (auth_user_str != NULL) {
        printf("Storing user to flash: %s\n", auth_user_str);
        store_char(key, auth_user_str);
        store_u32("auth_user_count", user_count + 1);

        free(auth_user_str);
        cJSON_Delete(user);
    } else {
        ESP_LOGE(TAG, "Failed to print JSON");
    }
}

cJSON* load_user_from_flash(uint32_t user_id) {
    char key[20];
    snprintf(key, sizeof(key), "auth_user_%lu", (unsigned long)user_id);

    char *auth_user_str = get_char(key);
    if (auth_user_str == NULL) {
        ESP_LOGE(TAG, "User not found");
        return NULL;
    }

    cJSON *user = cJSON_Parse(auth_user_str);
    free(auth_user_str);

    if (user == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return NULL;
    }

    return user;
}

void delete_char(char *key_to_delete) {
    // Parse key to get the user_id of the user to delete
    uint32_t user_id_to_delete;
    sscanf(key_to_delete, "auth_user_%lu", &user_id_to_delete);

    // Get the current user count
    uint32_t user_count = get_u32("auth_user_count", 0);
    if (user_id_to_delete > user_count || user_id_to_delete == 0) {
        printf("Invalid user ID. Cannot delete.\n");
        return;
    }

    // Shift down users
    for (uint32_t i = user_id_to_delete; i < user_count; i++) {
        cJSON *user_to_move = load_user_from_flash(i + 1);
        if (user_to_move != NULL) {
            // Convert the user object to a string
            char *auth_user_str = cJSON_PrintUnformatted(user_to_move);  // Allocated dynamically
            if (auth_user_str != NULL) {
                // Generate the key for the position to which the user will be moved
                char new_key[50];
                snprintf(new_key, sizeof(new_key), "auth_user_%lu", (unsigned long)i);
                
                // Store the moved user to the new position in flash
                store_char(new_key, auth_user_str);
                
                // Free the allocated memory
                free(auth_user_str);

                // Free cJSON object
                cJSON_Delete(user_to_move);
            } else {
                printf("Failed to print JSON\n");
            }
        }
    }

    // Delete the last user's data from flash as it should have been moved down
    char last_user_key[20];
    snprintf(last_user_key, sizeof(last_user_key), "auth_user_%lu", (unsigned long)user_count);
    // Add your code here to actually remove this key-value pair from flash
    // This may depend on your specific flash storage API

    // Update the user count in flash
    store_u32("auth_user_count", user_count - 1);
}

void sendUsers() {
    uint32_t user_count = get_u32("auth_user_count", 0);

    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) {
            continue;
        }

        cJSON *users_json = cJSON_CreateArray();
        cJSON_AddItemToArray(users_json, user);

        char *users_json_str = cJSON_PrintUnformatted(users_json);
        if (users_json_str == NULL) {
            cJSON_Delete(users_json);
            continue;
        }

        char msg[2000] = "";
        snprintf(msg, sizeof(msg), "{\"event_type\":\"load\", \"payload\":{\"services\":[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":{\"users\":%s}}]}}", users_json_str);

        addServerMessageToQueue(msg);

        free(users_json_str);
        cJSON_Delete(users_json);
    }
}

void addUser(char *uuid, char *name, char *pin) {
    uint32_t user_count = get_u32("auth_user_count", 0);
    
    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;
        
        cJSON *name_json = cJSON_GetObjectItem(user, "name");
        if (name_json != NULL && strcmp(name_json->valuestring, name) == 0) {
            printf("User with name %s already exists.\n", name);
            cJSON_Delete(user);
            return;
        }
        cJSON_Delete(user);
    }
    
    store_user_to_flash(uuid, name, pin);
    printf("User added: UUID = %s, Name = %s, Pin = %s\n", uuid, name, pin);
    sendUsers();
}

void modifyUser(char *uuid, char *newName, char *newPin) {
    uint32_t user_count = get_u32("auth_user_count", 0);

    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;

        cJSON *uuid_json = cJSON_GetObjectItem(user, "uuid");
        if (uuid_json != NULL && strcmp(uuid_json->valuestring, uuid) == 0) {
            store_user_to_flash(uuid, newName, newPin);
            cJSON_Delete(user);
            printf("User modified: New Name = %s, New Pin = %s\n", newName, newPin);
            sendUsers();
            return;
        }
        cJSON_Delete(user);
    }
    printf("No user with uuid %s found to modify.\n", uuid);
}

void removeUser(char *uuid) {
    printf("Removing user with uuid %s\n", uuid);
    uint32_t user_count = get_u32("auth_user_count", 0);

    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;

        cJSON *uuid_json = cJSON_GetObjectItem(user, "uuid");
        if (uuid_json != NULL && strcmp(uuid_json->valuestring, uuid) == 0) {
            char key[20];
            snprintf(key, sizeof(key), "auth_user_%lu", (unsigned long)i);
            delete_char(key);  // Assume delete_char removes key-value from flash

            // Decrement the user count
            store_u32("auth_user_count", user_count - 1);

            cJSON_Delete(user);
            printf("User with uuid %s removed.\n", uuid);
            sendUsers();
            return;
        }
        cJSON_Delete(user);
    }
    printf("No user with uuid %s found.\n", uuid);
}

char* find_pin_in_flash(const char* pin) {
    uint32_t user_count = get_u32("auth_user_count", 0);
    ESP_LOGI(TAG, "Total User Count: %u", (unsigned int)user_count);  // Logging total users
    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;
        
        cJSON *pin_json = cJSON_GetObjectItem(user, "pin");
        if (pin_json != NULL && strcmp(pin_json->valuestring, pin) == 0) {
            cJSON *name_json = cJSON_GetObjectItem(user, "name");
            char *name = name_json ? name_json->valuestring : "Unknown";
            ESP_LOGI(TAG, "Name Found: %s", name);  // Logging the name found
            cJSON_Delete(user);
            return name;
        }
        cJSON_Delete(user);
    }
    return NULL;
}


int is_pin_authorized(const char *incomingPin) {
    char* name = find_pin_in_flash(incomingPin);
    if (name) {
        ESP_LOGI(TAG, "%s was authorized using %s\n", name, incomingPin);
        return 1;
    }
    ESP_LOGI(TAG, "Not Authorized: %s\n", incomingPin);
    return 0;
}


void handle_authorize_message(cJSON * payload)
{ 
    char user[250];
    char uuid[250];
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
              sprintf(uuid, "%s", cJSON_GetObjectItem(value, "uuid")->valuestring);
              sprintf(name, "%s", cJSON_GetObjectItem(value, "name")->valuestring);
              sprintf(pin, "%s", cJSON_GetObjectItem(value, "pin")->valuestring);
            }
            addUser(uuid, name, pin);
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
  TaskHandle_t auth_service_task;
  xTaskCreate(&auth_service, "auth_service_task", 9 * 1000, NULL, 5, NULL);
}
