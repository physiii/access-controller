#define MAX_UIDS		400
#define MAX_UID_SIZE	50

#define MAX_USERS		400
#define MAX_UID_SIZE	50
#define MAX_NAME_SIZE   50
#define MAX_PIN_SIZE    10

#define MAX_SSID_SIZE     32
#define MAX_PASS_SIZE     64

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

cJSON *auth_users = NULL;

void addUser(char *uuid, char *name, char *pin) {
    uint32_t user_count = get_u32("auth_user_count", 0);

    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;
        
        cJSON *uuid_json = cJSON_GetObjectItem(user, "uuid");
        if (uuid_json != NULL && strcmp(uuid_json->valuestring, uuid) == 0) {
            ESP_LOGI(TAG, "User with UUID %s already exists.\n", uuid);
            cJSON_Delete(user);
            return;
        }
        cJSON_Delete(user);
    }
    store_user_to_flash(uuid, name, pin);
}

/* Copy PIN with '*' removed so keypads that send * between digits still match. */
static void pin_strip_star(const char *in, char *out, size_t out_size) {
    size_t j = 0;
    for (; *in != '\0' && j < out_size - 1; in++) {
        if (*in != '*') out[j++] = *in;
    }
    out[j] = '\0';
}

int is_pin_authorized(const char *incomingPin) {
    char* name = find_pin_in_flash(incomingPin);
    if (!name && incomingPin) {
        char stripped[MAX_PIN_SIZE];
        pin_strip_star(incomingPin, stripped, sizeof(stripped));
        if (stripped[0] != '\0')
            name = find_pin_in_flash(stripped);
    }
    char log_msg[1000];

    if (name) {
        snprintf(log_msg, sizeof(log_msg), 
             "{\"event_type\":\"log\",\"payload\":"
             "{\"service_id\":\"ac_1\", "
             "\"type\":\"access-control\", "
             "\"description\":\"%s was authorized using pin %s.\", "
             "\"event\":\"authentication\", "
             "\"value\":\"%s\"}"
             "}", name, incomingPin, incomingPin);
        addServerMessageToQueue(log_msg);
        ESP_LOGI(TAG, "%s was authorized using %s\n", name, incomingPin);

        free(name);

        return 1;
    }

    snprintf(log_msg, sizeof(log_msg), 
            "{\"event_type\":\"log\",\"payload\":"
            "{\"service_id\":\"ac_1\", "
            "\"type\":\"access-control\", "
            "\"description\":\"Invalid pin attempted %s.\", "
            "\"event\":\"authentication\", "
            "\"value\":\"%s\"}"
            "}", incomingPin, incomingPin);
    addServerMessageToQueue(log_msg);
    ESP_LOGI(TAG, "Not Authorized: %s\n", incomingPin);

    if (name) {
        free(name);
    }

    return 0;
}

void send_user_count() {
    uint32_t user_count = get_u32("auth_user_count", 0);
    char msg[1000] = "";
    snprintf(msg, sizeof(msg), "{\"event_type\":\"load\", \"payload\":{\"services\":[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":{\"user_count\": %lu}}]}}", user_count);
    addServerMessageToQueue(msg);
}

void sendInfo(void)
{
    char msg[200];
    sprintf(msg, "{\"eventType\":\"authorize\", \"payload\":{\"uuid\":\"%s\"}}", device_id);
    cJSON *json_msg = cJSON_Parse(msg);
    addClientMessageToQueue(json_msg);
    cJSON_Delete(json_msg);
}

void handle_authorize_message(cJSON * payload) {
    if (payload == NULL) return;

    cJSON *propertyItem = cJSON_GetObjectItem(payload,"property");
    char uuid[250];
    char property[250];

    if (propertyItem) {
        snprintf(property, sizeof(property), "%s", propertyItem->valuestring);

        if (strcmp(property, "getUserCount") == 0) {
            send_user_count();

        } else if (strcmp(property, "getUserByCount") == 0) {
            int count = 0;
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value) {
                count = value->valueint;
                cJSON *user = load_user_from_flash(count);
                if (user) {
                    char *user_json_str = cJSON_PrintUnformatted(user);
                    if (user_json_str) {
                        char msg[2000] = "";
                        snprintf(msg, sizeof(msg), "{\"event_type\":\"load\", \"payload\":{\"services\":[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":{\"user\":%s, \"count\": %d}}]}}", user_json_str, count);
                        addServerMessageToQueue(msg);
                        free(user_json_str);
                    }
                    cJSON_Delete(user);
                }
            }

        } else if (strcmp(property, "addUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value && cJSON_GetObjectItem(value, "name") && cJSON_GetObjectItem(value, "pin")) {
                sprintf(uuid, "%s", cJSON_GetObjectItem(value, "uuid")->valuestring);
                char name[MAX_NAME_SIZE];
                sprintf(name, "%s", cJSON_GetObjectItem(value, "name")->valuestring);
                char pin[MAX_PIN_SIZE];
                sprintf(pin, "%s", cJSON_GetObjectItem(value, "pin")->valuestring);
                addUser(uuid, name, pin);
            }

        } else if (strcmp(property, "modifyUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value) {
                sprintf(uuid, "%s", cJSON_GetObjectItem(value, "uuid")->valuestring);
                char newName[MAX_NAME_SIZE];
                sprintf(newName, "%s", cJSON_GetObjectItem(value, "newName")->valuestring);
                char newPin[MAX_PIN_SIZE];
                sprintf(newPin, "%s", cJSON_GetObjectItem(value, "newPin")->valuestring);
                modify_user_from_flash(uuid, newName, newPin);
            }

        } else if (strcmp(property, "removeUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value) {
                sprintf(uuid, "%s", value->valuestring);
                delete_user_from_flash(uuid);
            }
        }
    } else if (cJSON_GetObjectItem(payload,"uuid")) {
        const cJSON *uuid_item = cJSON_GetObjectItem(payload, "uuid");
        if (cJSON_IsString(uuid_item) && uuid_item->valuestring) {
            if (strcmp(device_id, uuid_item->valuestring) != 0) {
                snprintf(device_id, sizeof(device_id), "%s", uuid_item->valuestring);
                store_char("device_id", device_id);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                esp_restart();
            } else {
                ESP_LOGI(TAG, "UUID unchanged, skipping restart");
            }
        }

    } else if (cJSON_GetObjectItem(payload,"token")) {
        const cJSON *token_item = cJSON_GetObjectItem(payload, "token");
        if (cJSON_IsString(token_item) && token_item->valuestring) {
            if (strcmp(token, token_item->valuestring) != 0) {
                snprintf(token, sizeof(token), "%s", token_item->valuestring);
                store_char("token", token);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                esp_restart();
            } else {
                ESP_LOGI(TAG, "Token unchanged, skipping restart");
            }
        }

    } else if (cJSON_GetObjectItem(payload,"serverIp")) {
        char serverIp[32] = "";
        char serverPort[8] = "";
        if (cJSON_GetObjectItem(payload, "serverIp") && cJSON_GetObjectItem(payload, "serverPort")) {
            snprintf(serverIp, sizeof(serverIp), "%s", cJSON_GetObjectItem(payload, "serverIp")->valuestring);
            snprintf(serverPort, sizeof(serverPort), "%s", cJSON_GetObjectItem(payload, "serverPort")->valuestring);
            esp_err_t err = store_server_info_to_flash(serverIp, serverPort);
            if (err == ESP_OK) {
                vTaskDelay(100 / portTICK_PERIOD_MS);
                esp_restart();
            } else {
                ESP_LOGE(TAG, "Failed to update server settings: %s", esp_err_to_name(err));
            }
        }

    } else if (cJSON_GetObjectItem(payload,"wifiName")) {
        char wifiName[MAX_SSID_SIZE] = "";
        char wifiPassword[MAX_PASS_SIZE] = "";
        if (cJSON_GetObjectItem(payload, "wifiName") && cJSON_GetObjectItem(payload, "wifiPassword")) {
            snprintf(wifiName, sizeof(wifiName), "%s", cJSON_GetObjectItem(payload, "wifiName")->valuestring);
            snprintf(wifiPassword, sizeof(wifiPassword), "%s", cJSON_GetObjectItem(payload, "wifiPassword")->valuestring);
            esp_err_t err = store_wifi_credentials_to_flash(wifiName, wifiPassword);
            if (err == ESP_OK) {
                vTaskDelay(200 / portTICK_PERIOD_MS);
                esp_restart();
            } else {
                ESP_LOGE(TAG, "Failed to update WiFi credentials: %s", esp_err_to_name(err));
            }
        }

    } else if (cJSON_GetObjectItem(payload,"getInfo")) {
		sendInfo();
	}

    cJSON_Delete(payload);
}

static void auth_service (void *pvParameter)
{
  uint32_t io_num;
  uint8_t r;

  while (1) {
    handle_authorize_message(checkServiceMessageByType("users"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "addUser"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "modifyUser"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "removeUser"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "getUserCount"));
    handle_authorize_message(checkServiceMessageByAction("ac_1", "getUserByCount"));
    handle_authorize_message(checkServiceMessageByType("setWifiCredentials"));
    handle_authorize_message(checkServiceMessageByType("setServerInfo"));
    handle_authorize_message(checkServiceMessageByType("getInfo"));
    handle_authorize_message(checkServiceMessageByKey("uuid"));
    handle_authorize_message(checkServiceMessageByKey("token"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void auth_main()
{
  ESP_LOGI(TAG, "starting auth service");
  TaskHandle_t auth_service_task;
  xTaskCreate(&auth_service, "auth_service_task", 25 * 1000, NULL, 5, NULL);
}
