#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_random.h"
#include "cJSON.h"
#include "freertos/semphr.h"

#include "automation.h"
#include "wiegand.h"
#include "wiegand_registry.h"
#include "rf_registry.h"
#include "store.h"
#include "esp_system.h"

static const char *API_TAG = "api_server";

#define STATE_RESPONSE_BUFFER_SIZE 24576

static SemaphoreHandle_t s_response_mutex;
static char s_state_response_buffer[STATE_RESPONSE_BUFFER_SIZE];

extern cJSON *lock_state_snapshot(void);
extern cJSON *exit_state_snapshot(void);
extern cJSON *fob_state_snapshot(void);
extern cJSON *keypad_state_snapshot(void);
extern cJSON *wiegand_state_snapshot(void);
extern cJSON *system_logs_snapshot(void);

extern void handle_lock_message(cJSON *payload);
extern void handle_exit_message(cJSON *payload);
extern void handle_fob_message(cJSON *payload);
extern void handle_keypad_message(cJSON *payload);
extern void handle_authorize_message(cJSON *payload);

extern char device_id[100];
extern uint32_t get_u32(const char *key, uint32_t default_value);
extern cJSON *load_user_from_flash(uint32_t user_id);
extern void store_user_to_flash(char *uuid, char *name, char *pin);
extern void delete_user_from_flash(const char *uuid);
extern void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin);

// Forward declaration
static cJSON *keypad_users_snapshot(void);

static cJSON *build_state_snapshot(void) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON *device = cJSON_CreateObject();
    if (device) {
        cJSON_AddStringToObject(device, "uuid", device_id);
        cJSON_AddItemToObject(root, "device", device);
    }

    cJSON *locks = lock_state_snapshot();
    if (!locks) {
        locks = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "locks", locks);

    cJSON *exits = exit_state_snapshot();
    if (!exits) {
        exits = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "exits", exits);

    cJSON *fobs = fob_state_snapshot();
    if (!fobs) {
        fobs = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "fobs", fobs);

    cJSON *keypads = keypad_state_snapshot();
    if (!keypads) {
        keypads = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "keypads", keypads);

    cJSON *wiegand = wiegand_state_snapshot();
    if (!wiegand) {
        wiegand = cJSON_CreateObject();
    }
    cJSON_AddItemToObject(root, "wiegand", wiegand);

    cJSON *rf = rf_state_snapshot();
    if (!rf) {
        rf = cJSON_CreateObject();
    }
    cJSON_AddItemToObject(root, "rf", rf);

    cJSON *logs = system_logs_snapshot();
    if (!logs) {
        logs = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "logs", logs);

    /* Wi-Fi state */
    cJSON *wifi = cJSON_CreateObject();
    if (wifi) {
        char ssid[32] = {0};
        char pwd[64] = {0};
        load_wifi_credentials_from_flash(ssid, pwd);
        cJSON_AddStringToObject(wifi, "active_ssid", ssid);
        cJSON *list = wifi_list_snapshot();
        if (list) {
            cJSON_AddItemToObject(wifi, "networks", list);
        }
        cJSON_AddItemToObject(root, "wifi", wifi);
    }

    // Add keypad PIN users
    cJSON *keypad_users = keypad_users_snapshot();
    if (!keypad_users) {
        keypad_users = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "keypadUsers", keypad_users);

    return root;
}

static esp_err_t send_json_response(httpd_req_t *req, cJSON *json) {
    if (!json) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build JSON response");
    }

    bool used_preallocated = false;
    bool mutex_held = false;
    char *resp_str = NULL;

    if (!s_response_mutex) {
        s_response_mutex = xSemaphoreCreateMutex();
    }

    if (s_response_mutex && xSemaphoreTake(s_response_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        mutex_held = true;
        if (cJSON_PrintPreallocated(json, s_state_response_buffer, STATE_RESPONSE_BUFFER_SIZE, false)) {
            resp_str = s_state_response_buffer;
            used_preallocated = true;
        }
    }

    if (!resp_str) {
        resp_str = cJSON_PrintUnformatted(json);
        if (!resp_str) {
            if (mutex_held) {
                xSemaphoreGive(s_response_mutex);
            }
            cJSON_Delete(json);
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to serialise JSON");
        }
    }

    size_t resp_len = strlen(resp_str);
    ESP_LOGI(API_TAG, "state response size=%zu (prealloc=%s)", resp_len, used_preallocated ? "yes" : "no");

    httpd_resp_set_type(req, "application/json");
    esp_err_t result = httpd_resp_send(req, resp_str, resp_len);
    cJSON_Delete(json);
    if (!used_preallocated) {
        free(resp_str);
    }
    if (mutex_held) {
        xSemaphoreGive(s_response_mutex);
    }
    return result;
}

static esp_err_t read_json_body(httpd_req_t *req, cJSON **out_payload) {
    if (!out_payload) {
        return ESP_ERR_INVALID_ARG;
    }
    const size_t max_len = 4096;
    int total_len = req->content_len;
    if (total_len <= 0 || total_len > (int)max_len) {
        ESP_LOGE(API_TAG, "Invalid JSON body length: %d", total_len);
        return ESP_ERR_INVALID_SIZE;
    }

    char *buf = malloc(total_len + 1);
    if (!buf) {
        return ESP_ERR_NO_MEM;
    }

    int received = 0;
    while (received < total_len) {
        int ret = httpd_req_recv(req, buf + received, total_len - received);
        if (ret <= 0) {
            free(buf);
            return ESP_FAIL;
        }
        received += ret;
    }
    buf[total_len] = '\0';

    cJSON *payload = cJSON_Parse(buf);
    free(buf);
    if (!payload) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    *out_payload = payload;
    return ESP_OK;
}

static esp_err_t api_state_get_handler(httpd_req_t *req) {
    return send_json_response(req, build_state_snapshot());
}

static esp_err_t send_wiegand_state_response(httpd_req_t *req) {
    return send_json_response(req, wiegand_state_snapshot());
}

static esp_err_t api_wiegand_get_handler(httpd_req_t *req) {
    ESP_LOGI(API_TAG, "Wiegand state requested");
    return send_wiegand_state_response(req);
}

static esp_err_t api_wiegand_register_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        ESP_LOGW(API_TAG, "Invalid Wiegand register payload (%s)", esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON payload");
    }

    int channel = 0;
    const cJSON *channel_item = cJSON_GetObjectItemCaseSensitive(payload, "channel");
    if (cJSON_IsNumber(channel_item)) {
        channel = (int)channel_item->valuedouble;
    }

    ESP_LOGI(API_TAG, "Starting Wiegand registration (channel=%d)", channel);
    err = wiegand_registration_start((uint8_t)channel);
    cJSON_Delete(payload);

    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(API_TAG, "Wiegand registration already active");
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Registration already active");
    }
    if (err != ESP_OK) {
        ESP_LOGE(API_TAG, "Failed to start Wiegand registration (%s)", esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start registration");
    }

    return send_wiegand_state_response(req);
}

static esp_err_t api_wiegand_stop_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        ESP_LOGW(API_TAG, "Invalid Wiegand stop payload (%s)", esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON payload");
    }

    bool promote = false;
    const cJSON *promote_item = cJSON_GetObjectItemCaseSensitive(payload, "promote");
    if (cJSON_IsBool(promote_item)) {
        promote = cJSON_IsTrue(promote_item);
    }
    cJSON_Delete(payload);

    ESP_LOGI(API_TAG, "Stopping Wiegand registration (promote=%s)", promote ? "true" : "false");
    err = wiegand_registration_stop(promote);
    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(API_TAG, "Attempted to stop inactive Wiegand registration");
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Registration is not active");
    }
    if (err != ESP_OK) {
        ESP_LOGE(API_TAG, "Failed to stop Wiegand registration (%s)", esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to stop registration");
    }

    return send_wiegand_state_response(req);
}

static esp_err_t api_wiegand_rename_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        ESP_LOGW(API_TAG, "Invalid Wiegand rename payload (%s)", esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON payload");
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(payload, "id");
    const cJSON *name_item = cJSON_GetObjectItemCaseSensitive(payload, "name");
    const char *id = (cJSON_IsString(id_item) && id_item->valuestring) ? id_item->valuestring : NULL;
    const char *name = (cJSON_IsString(name_item) && name_item->valuestring) ? name_item->valuestring : NULL;

    if (!id || id[0] == '\0' || !name || name[0] == '\0') {
        cJSON_Delete(payload);
        ESP_LOGW(API_TAG, "Wiegand rename missing id or name");
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Both id and name are required");
    }
    if (strlen(name) >= WIEGAND_USER_NAME_MAX) {
        cJSON_Delete(payload);
        ESP_LOGW(API_TAG, "Wiegand rename name too long (%u >= %d)", (unsigned)strlen(name), WIEGAND_USER_NAME_MAX);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Name exceeds maximum length");
    }

    ESP_LOGI(API_TAG, "Renaming Wiegand user %s to '%s'", id, name);
    err = wiegand_registry_update_name(id, name);
    cJSON_Delete(payload);

    if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(API_TAG, "Wiegand user not found: %s", id);
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "User not found");
    }
    if (err != ESP_OK) {
        ESP_LOGE(API_TAG, "Failed to rename Wiegand user %s (%s)", id, esp_err_to_name(err));
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to rename user");
    }

    return send_wiegand_state_response(req);
}

static esp_err_t handle_json_post(httpd_req_t *req, void (*handler)(cJSON *), cJSON *(*state_builder)(void)) {
    const size_t max_len = 2048;
    int total_len = req->content_len;
    if (total_len <= 0 || total_len > (int)max_len) {
        ESP_LOGE(API_TAG, "Invalid content length: %d", total_len);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
    }

    char *buf = malloc(total_len + 1);
    if (!buf) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    }

    int received = 0;
    while (received < total_len) {
        int ret = httpd_req_recv(req, buf + received, total_len - received);
        if (ret <= 0) {
            free(buf);
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read request body");
        }
        received += ret;
    }
    buf[total_len] = '\0';

    cJSON *payload = cJSON_Parse(buf);
    free(buf);

    if (!payload) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    handler(cJSON_Duplicate(payload, 1));
    cJSON_Delete(payload);

    cJSON *state = state_builder ? state_builder() : cJSON_CreateObject();
    if (!state) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build state");
    }
    return send_json_response(req, state);
}

static void lock_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_lock_message(payload);
    }
}

static void exit_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_exit_message(payload);
    }
}

static void fob_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_fob_message(payload);
    }
}

static void keypad_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_keypad_message(payload);
    }
}

static esp_err_t api_lock_post_handler(httpd_req_t *req) {
    return handle_json_post(req, lock_message_wrapper, lock_state_snapshot);
}

static esp_err_t api_exit_post_handler(httpd_req_t *req) {
    return handle_json_post(req, exit_message_wrapper, exit_state_snapshot);
}

static esp_err_t api_fob_post_handler(httpd_req_t *req) {
    return handle_json_post(req, fob_message_wrapper, fob_state_snapshot);
}

static esp_err_t api_keypad_post_handler(httpd_req_t *req) {
    return handle_json_post(req, keypad_message_wrapper, keypad_state_snapshot);
}

static esp_err_t api_wifi_post_handler(httpd_req_t *req) {
    return handle_json_post(req, handle_authorize_message, build_state_snapshot);
}

static esp_err_t api_server_post_handler(httpd_req_t *req) {
    return handle_json_post(req, handle_authorize_message, build_state_snapshot);
}

static esp_err_t api_wifi_get_handler(httpd_req_t *req) {
    return send_json_response(req, build_state_snapshot());
}

static esp_err_t api_wifi_add_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    const cJSON *ssid_item = cJSON_GetObjectItemCaseSensitive(payload, "ssid");
    const cJSON *pass_item = cJSON_GetObjectItemCaseSensitive(payload, "password");
    const char *ssid = cJSON_IsString(ssid_item) ? ssid_item->valuestring : NULL;
    const char *pwd = cJSON_IsString(pass_item) ? pass_item->valuestring : "";
    if (!ssid || ssid[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "ssid required");
    }
    err = wifi_list_add(ssid, pwd);
    cJSON_Delete(payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save WiFi");
    }
    httpd_resp_sendstr(req, "OK");
    esp_restart();
    return ESP_OK;
}

static esp_err_t api_wifi_delete_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    const cJSON *ssid_item = cJSON_GetObjectItemCaseSensitive(payload, "ssid");
    const char *ssid = cJSON_IsString(ssid_item) ? ssid_item->valuestring : NULL;
    if (!ssid || ssid[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "ssid required");
    }
    err = wifi_list_delete(ssid);
    cJSON_Delete(payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete WiFi");
    }
    return send_json_response(req, build_state_snapshot());
}

static esp_err_t api_wifi_connect_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    const cJSON *ssid_item = cJSON_GetObjectItemCaseSensitive(payload, "ssid");
    const char *ssid = cJSON_IsString(ssid_item) ? ssid_item->valuestring : NULL;
    if (!ssid || ssid[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "ssid required");
    }
    err = wifi_list_set_active(ssid);
    cJSON_Delete(payload);
    if (err == ESP_ERR_NOT_FOUND) {
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "ssid not found");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to activate WiFi");
    }
    httpd_resp_sendstr(req, "OK");
    esp_restart();
    return ESP_OK;
}


static esp_err_t api_favicon_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/x-icon");
    return httpd_resp_send(req, NULL, 0);
}

// Build JSON array of keypad PIN users
static cJSON *keypad_users_snapshot(void) {
    cJSON *array = cJSON_CreateArray();
    if (!array) return NULL;

    uint32_t user_count = get_u32("auth_user_count", 0);
    for (uint32_t i = 0; i < user_count; i++) {
        cJSON *user = load_user_from_flash(i + 1);
        if (user) {
            cJSON_AddItemToArray(array, user);
        }
    }
    return array;
}

// Generate a random UUID
static void generate_uuid(char *buf, size_t len) {
    const char *hex = "0123456789abcdef";
    for (size_t i = 0; i + 1 < len; i++) {
        buf[i] = hex[esp_random() % 16];
    }
    buf[len - 1] = '\0';
}

// POST /api/keypad/user - Add new PIN user
static esp_err_t api_keypad_user_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *name_item = cJSON_GetObjectItemCaseSensitive(payload, "name");
    const cJSON *pin_item = cJSON_GetObjectItemCaseSensitive(payload, "pin");

    const char *name = cJSON_IsString(name_item) ? name_item->valuestring : NULL;
    const char *pin = cJSON_IsString(pin_item) ? pin_item->valuestring : NULL;

    if (!name || !pin || strlen(pin) < 4 || strlen(pin) > 6) {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Name and PIN (4-6 digits) required");
    }

    char uuid[33];
    generate_uuid(uuid, sizeof(uuid));

    ESP_LOGI(API_TAG, "Adding keypad user: name=%s, pin=****", name);
    store_user_to_flash(uuid, (char *)name, (char *)pin);
    cJSON_Delete(payload);

    cJSON *users = keypad_users_snapshot();
    return send_json_response(req, users);
}

// DELETE /api/keypad/user - Delete PIN user by UUID
static esp_err_t api_keypad_user_delete_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *uuid_item = cJSON_GetObjectItemCaseSensitive(payload, "uuid");
    const char *uuid = cJSON_IsString(uuid_item) ? uuid_item->valuestring : NULL;

    if (!uuid || uuid[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "UUID required");
    }

    ESP_LOGI(API_TAG, "Deleting keypad user: uuid=%s", uuid);
    delete_user_from_flash(uuid);
    cJSON_Delete(payload);

    cJSON *users = keypad_users_snapshot();
    return send_json_response(req, users);
}

// PUT /api/keypad/user - Update PIN user name
static esp_err_t api_keypad_user_put_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *uuid_item = cJSON_GetObjectItemCaseSensitive(payload, "uuid");
    const cJSON *name_item = cJSON_GetObjectItemCaseSensitive(payload, "name");

    const char *uuid = cJSON_IsString(uuid_item) ? uuid_item->valuestring : NULL;
    const char *name = cJSON_IsString(name_item) ? name_item->valuestring : NULL;

    if (!uuid || !name) {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "UUID and name required");
    }

    ESP_LOGI(API_TAG, "Updating keypad user: uuid=%s, name=%s", uuid, name);
    modify_user_from_flash(uuid, name, NULL);
    cJSON_Delete(payload);

    cJSON *users = keypad_users_snapshot();
    return send_json_response(req, users);
}

// DELETE /api/wiegand/delete - Remove Wiegand user
static esp_err_t api_wiegand_delete_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(payload, "id");
    const char *id = cJSON_IsString(id_item) ? id_item->valuestring : NULL;

    if (!id || id[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "User ID required");
    }

    ESP_LOGI(API_TAG, "Deleting Wiegand user: id=%s", id);
    err = wiegand_registry_remove(id);
    cJSON_Delete(payload);

    if (err == ESP_ERR_NOT_FOUND) {
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "User not found");
    }

    return send_wiegand_state_response(req);
}

/* RF remote fobs */
static esp_err_t send_rf_state_response(httpd_req_t *req) {
    return send_json_response(req, rf_state_snapshot());
}

static esp_err_t api_rf_get_handler(httpd_req_t *req) {
    ESP_LOGI(API_TAG, "RF state requested");
    return send_rf_state_response(req);
}

static esp_err_t api_rf_register_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    cJSON_Delete(payload);

    err = rf_registration_start();
    if (err == ESP_ERR_INVALID_STATE) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Registration already active");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start registration");
    }
    return send_rf_state_response(req);
}

static esp_err_t api_rf_stop_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    cJSON_Delete(payload);

    err = rf_registration_stop();
    if (err == ESP_ERR_INVALID_STATE) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Registration is not active");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to stop registration");
    }
    return send_rf_state_response(req);
}

static esp_err_t api_rf_rename_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(payload, "id");
    const cJSON *name_item = cJSON_GetObjectItemCaseSensitive(payload, "name");
    const char *id = cJSON_IsString(id_item) ? id_item->valuestring : NULL;
    const char *name = cJSON_IsString(name_item) ? name_item->valuestring : NULL;

    if (!id || !name || name[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "id and name required");
    }

    err = rf_registry_update_name(id, name);
    cJSON_Delete(payload);
    if (err == ESP_ERR_NOT_FOUND) {
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "RF code not found");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to update name");
    }
    return send_rf_state_response(req);
}

static esp_err_t api_rf_delete_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(payload, "id");
    const char *id = cJSON_IsString(id_item) ? id_item->valuestring : NULL;
    if (!id || id[0] == '\0') {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "id required");
    }

    err = rf_registry_remove(id);
    cJSON_Delete(payload);
    if (err == ESP_ERR_NOT_FOUND) {
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "RF code not found");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete RF code");
    }
    return send_rf_state_response(req);
}

static esp_err_t api_rf_config_post_handler(httpd_req_t *req) {
    cJSON *payload = NULL;
    esp_err_t err = read_json_body(req, &payload);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }
    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(payload, "id");
    const cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(payload, "mode");
    const cJSON *ch_item = cJSON_GetObjectItemCaseSensitive(payload, "channel_mask");
    const cJSON *exit_item = cJSON_GetObjectItemCaseSensitive(payload, "exit_seconds");
    const cJSON *alert_item = cJSON_GetObjectItemCaseSensitive(payload, "alert");
    const char *id = cJSON_IsString(id_item) ? id_item->valuestring : NULL;
    const char *mode = cJSON_IsString(mode_item) ? mode_item->valuestring : NULL;
    int ch_mask = cJSON_IsNumber(ch_item) ? (int)ch_item->valuedouble : 0;
    int exit_s = cJSON_IsNumber(exit_item) ? (int)exit_item->valuedouble : 0;
    bool alert = alert_item ? cJSON_IsTrue(alert_item) : true;

    if (!id || !mode || ch_mask <= 0) {
        cJSON_Delete(payload);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "id, mode, channel_mask required");
    }

    err = rf_registry_update_config(id, mode, ch_mask, exit_s, alert);
    cJSON_Delete(payload);
    if (err == ESP_ERR_NOT_FOUND) {
        return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "RF code not found");
    }
    if (err == ESP_ERR_INVALID_ARG) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid mode/channel");
    }
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to update config");
    }
    return send_rf_state_response(req);
}

void register_api_routes(httpd_handle_t server) {
    httpd_uri_t state = {
        .uri = "/api/state",
        .method = HTTP_GET,
        .handler = api_state_get_handler,
    };
    httpd_register_uri_handler(server, &state);

    httpd_uri_t wiegand_get = {
        .uri = "/api/wiegand",
        .method = HTTP_GET,
        .handler = api_wiegand_get_handler,
    };
    httpd_register_uri_handler(server, &wiegand_get);

    httpd_uri_t wiegand_register_post = {
        .uri = "/api/wiegand/register",
        .method = HTTP_POST,
        .handler = api_wiegand_register_post_handler,
    };
    httpd_register_uri_handler(server, &wiegand_register_post);

    httpd_uri_t wiegand_stop_post = {
        .uri = "/api/wiegand/stop",
        .method = HTTP_POST,
        .handler = api_wiegand_stop_post_handler,
    };
    httpd_register_uri_handler(server, &wiegand_stop_post);

    httpd_uri_t wiegand_rename_post = {
        .uri = "/api/wiegand/rename",
        .method = HTTP_POST,
        .handler = api_wiegand_rename_post_handler,
    };
    httpd_register_uri_handler(server, &wiegand_rename_post);

    httpd_uri_t wiegand_delete_post = {
        .uri = "/api/wiegand/delete",
        .method = HTTP_POST,
        .handler = api_wiegand_delete_post_handler,
    };
    httpd_register_uri_handler(server, &wiegand_delete_post);

    httpd_uri_t rf_get = {
        .uri = "/api/rf",
        .method = HTTP_GET,
        .handler = api_rf_get_handler,
    };
    httpd_register_uri_handler(server, &rf_get);

    httpd_uri_t rf_register_post = {
        .uri = "/api/rf/register",
        .method = HTTP_POST,
        .handler = api_rf_register_post_handler,
    };
    httpd_register_uri_handler(server, &rf_register_post);

    httpd_uri_t rf_stop_post = {
        .uri = "/api/rf/stop",
        .method = HTTP_POST,
        .handler = api_rf_stop_post_handler,
    };
    httpd_register_uri_handler(server, &rf_stop_post);

    httpd_uri_t rf_rename_post = {
        .uri = "/api/rf/rename",
        .method = HTTP_POST,
        .handler = api_rf_rename_post_handler,
    };
    httpd_register_uri_handler(server, &rf_rename_post);

    httpd_uri_t rf_delete_post = {
        .uri = "/api/rf/delete",
        .method = HTTP_POST,
        .handler = api_rf_delete_post_handler,
    };
    httpd_register_uri_handler(server, &rf_delete_post);

    httpd_uri_t rf_config_post = {
        .uri = "/api/rf/config",
        .method = HTTP_POST,
        .handler = api_rf_config_post_handler,
    };
    httpd_register_uri_handler(server, &rf_config_post);

    httpd_uri_t keypad_user_post = {
        .uri = "/api/keypad/user",
        .method = HTTP_POST,
        .handler = api_keypad_user_post_handler,
    };
    httpd_register_uri_handler(server, &keypad_user_post);

    httpd_uri_t keypad_user_delete = {
        .uri = "/api/keypad/user",
        .method = HTTP_DELETE,
        .handler = api_keypad_user_delete_handler,
    };
    httpd_register_uri_handler(server, &keypad_user_delete);

    httpd_uri_t keypad_user_put = {
        .uri = "/api/keypad/user",
        .method = HTTP_PUT,
        .handler = api_keypad_user_put_handler,
    };
    httpd_register_uri_handler(server, &keypad_user_put);

    httpd_uri_t lock_post = {
        .uri = "/api/lock",
        .method = HTTP_POST,
        .handler = api_lock_post_handler,
    };
    httpd_register_uri_handler(server, &lock_post);

    httpd_uri_t exit_post = {
        .uri = "/api/exit",
        .method = HTTP_POST,
        .handler = api_exit_post_handler,
    };
    httpd_register_uri_handler(server, &exit_post);

    httpd_uri_t fob_post = {
        .uri = "/api/fob",
        .method = HTTP_POST,
        .handler = api_fob_post_handler,
    };
    httpd_register_uri_handler(server, &fob_post);

    httpd_uri_t keypad_post = {
        .uri = "/api/keypad",
        .method = HTTP_POST,
        .handler = api_keypad_post_handler,
    };
    httpd_register_uri_handler(server, &keypad_post);

    httpd_uri_t wifi_post = {
        .uri = "/api/wifi",
        .method = HTTP_POST,
        .handler = api_wifi_post_handler,
    };
    httpd_register_uri_handler(server, &wifi_post);

    httpd_uri_t wifi_get = {
        .uri = "/api/wifi",
        .method = HTTP_GET,
        .handler = api_wifi_get_handler,
    };
    httpd_register_uri_handler(server, &wifi_get);

    httpd_uri_t wifi_add = {
        .uri = "/api/wifi/add",
        .method = HTTP_POST,
        .handler = api_wifi_add_post_handler,
    };
    httpd_register_uri_handler(server, &wifi_add);

    httpd_uri_t wifi_delete = {
        .uri = "/api/wifi/delete",
        .method = HTTP_POST,
        .handler = api_wifi_delete_post_handler,
    };
    httpd_register_uri_handler(server, &wifi_delete);

    httpd_uri_t wifi_connect = {
        .uri = "/api/wifi/connect",
        .method = HTTP_POST,
        .handler = api_wifi_connect_post_handler,
    };
    httpd_register_uri_handler(server, &wifi_connect);

    httpd_uri_t server_post = {
        .uri = "/api/server",
        .method = HTTP_POST,
        .handler = api_server_post_handler,
    };
    httpd_register_uri_handler(server, &server_post);

    httpd_uri_t favicon = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = api_favicon_handler,
    };
    httpd_register_uri_handler(server, &favicon);
}

