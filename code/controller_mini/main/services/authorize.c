#include "authorize.h"
#include "store.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "automation.h"

// Log AUTH_TAG
static const char *AUTH_TAG = "Authorize";

// Global variables
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
cJSON *auth_users = NULL;

void addUser(char *uuid, char *name, char *pin) {
    uint32_t user_count = get_u32("auth_user_count", 0);
    for (uint32_t i = 1; i <= user_count; i++) {
        cJSON *user = load_user_from_flash(i);
        if (user == NULL) continue;

        cJSON *uuid_json = cJSON_GetObjectItem(user, "uuid");
        if (uuid_json != NULL && strcmp(uuid_json->valuestring, uuid) == 0) {
            ESP_LOGI(AUTH_TAG, "User with UUID %s already exists.\n", uuid);
            cJSON_Delete(user);
            return;
        }
        cJSON_Delete(user);
    }
    store_user_to_flash(uuid, name, pin);
}

int is_pin_authorized(const char *incomingPin) {
    char *name = find_pin_in_flash(incomingPin);
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
        ESP_LOGI(AUTH_TAG, "%s was authorized using %s\n", name, incomingPin);

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
    ESP_LOGI(AUTH_TAG, "Not Authorized: %s\n", incomingPin);

    return 0;
}

void send_user_count() {
    uint32_t user_count = get_u32("auth_user_count", 0);
    char msg[1000];
    snprintf(msg, sizeof(msg), "{\"event_type\":\"load\", \"payload\":{\"services\":[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":{\"user_count\": %lu}}]}}", user_count);
    addServerMessageToQueue(msg);
}

void sendInfo() {
    char msg[200];
    snprintf(msg, sizeof(msg), "{\"eventType\":\"authorize\", \"payload\":{\"uuid\":\"%s\"}}", device_id);
    addClientMessageToQueue(msg);
}

void handle_authorize_message(cJSON *payload) {
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
                        char msg[2000];
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
                snprintf(uuid, sizeof(uuid), "%s", cJSON_GetObjectItem(value, "uuid")->valuestring);
                char name[MAX_NAME_SIZE];
                snprintf(name, sizeof(name), "%s", cJSON_GetObjectItem(value, "name")->valuestring);
                char pin[MAX_PIN_SIZE];
                snprintf(pin, sizeof(pin), "%s", cJSON_GetObjectItem(value, "pin")->valuestring);
                addUser(uuid, name, pin);
            }

        } else if (strcmp(property, "modifyUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value) {
                snprintf(uuid, sizeof(uuid), "%s", cJSON_GetObjectItem(value, "uuid")->valuestring);
                char newName[MAX_NAME_SIZE];
                snprintf(newName, sizeof(newName), "%s", cJSON_GetObjectItem(value, "newName")->valuestring);
                char newPin[MAX_PIN_SIZE];
                snprintf(newPin, sizeof(newPin), "%s", cJSON_GetObjectItem(value, "newPin")->valuestring);
                modify_user_from_flash(uuid, newName, newPin);
            }

        } else if (strcmp(property, "removeUser") == 0) {
            cJSON *value = cJSON_GetObjectItem(payload, "value");
            if (value) {
                snprintf(uuid, sizeof(uuid), "%s", value->valuestring);
                delete_user_from_flash(uuid);
            }
        }
    } else if (cJSON_GetObjectItem(payload,"uuid")) {
        snprintf(device_id, sizeof(device_id), "%s", cJSON_GetObjectItem(payload,"uuid")->valuestring);
        save_string_to_store("device_id", device_id);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        esp_restart();
    } else if (cJSON_GetObjectItem(payload,"token")) {
        snprintf(token, sizeof(token), "%s", cJSON_GetObjectItem(payload,"token")->valuestring);
        save_string_to_store("token", token);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        esp_restart();

    } else if (cJSON_GetObjectItem(payload,"serverIp")) {
        char serverIp[32] = "";
        char serverPort[8] = "";
        if (cJSON_GetObjectItem(payload, "serverIp") && cJSON_GetObjectItem(payload, "serverPort")) {
            snprintf(serverIp, sizeof(serverIp), "%s", cJSON_GetObjectItem(payload, "serverIp")->valuestring);
            snprintf(serverPort, sizeof(serverPort), "%s", cJSON_GetObjectItem(payload, "serverPort")->valuestring);
            save_string_to_store("serverIp", serverIp);
            save_string_to_store("serverPort", serverPort);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            esp_restart();
        }

    } else if (cJSON_GetObjectItem(payload,"wifiName")) {
        ESP_LOGI(AUTH_TAG, "Received WiFi credentials message");
        char wifiName[MAX_SSID_SIZE] = "";
        char wifiPassword[MAX_PASS_SIZE] = "";
        if (cJSON_GetObjectItem(payload, "wifiName") && cJSON_GetObjectItem(payload, "wifiPassword")) {
            snprintf(wifiName, sizeof(wifiName), "%s", cJSON_GetObjectItem(payload, "wifiName")->valuestring);
            snprintf(wifiPassword, sizeof(wifiPassword), "%s", cJSON_GetObjectItem(payload, "wifiPassword")->valuestring);
            ESP_LOGI(AUTH_TAG, "Saving WiFi credentials: SSID='%s', Password='%s'", wifiName, wifiPassword);
            save_string_to_store("wifiName", wifiName);
            save_string_to_store("wifiPassword", wifiPassword);
            
            // Send confirmation message before restart
            char confirm_msg[256];
            snprintf(confirm_msg, sizeof(confirm_msg), 
                     "{\"eventType\":\"wifiConfirm\", \"payload\":{\"status\":\"saved\", \"message\":\"WiFi credentials saved. Restarting...\"}}");
            addClientMessageToQueue(confirm_msg);
            
            ESP_LOGI(AUTH_TAG, "WiFi credentials saved, restarting in 500ms...");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            esp_restart();
        }

    } else if (cJSON_GetObjectItem(payload,"getInfo")) {
        sendInfo();
    }

    cJSON_Delete(payload);
}

static void auth_service(void *pvParameter) {
    while (1) {
        handle_authorize_message(checkServiceMessageByAction("ac_1", "addUser"));
        handle_authorize_message(checkServiceMessageByAction("ac_1", "modifyUser"));
        handle_authorize_message(checkServiceMessageByAction("ac_1", "removeUser"));
        handle_authorize_message(checkServiceMessageByAction("ac_1", "getUserCount"));
        handle_authorize_message(checkServiceMessageByAction("ac_1", "getUserByCount"));
        
        cJSON *wifi_msg = checkServiceMessage("setWifiCredentials");
        if (wifi_msg) {
            ESP_LOGI(AUTH_TAG, "Processing setWifiCredentials message");
            handle_authorize_message(wifi_msg);
        }
        
        handle_authorize_message(checkServiceMessage("setServerInfo"));
        handle_authorize_message(checkServiceMessage("getInfo"));
        handle_authorize_message(checkServiceMessageByKey("uuid"));
        handle_authorize_message(checkServiceMessageByKey("token"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void auth_main() {
    ESP_LOGI(AUTH_TAG, "starting auth service");
    xTaskCreate(auth_service, "auth_service_task", 25 * 1000, NULL, 5, NULL);
}
