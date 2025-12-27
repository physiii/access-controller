#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "esp_timer.h"
#include "automation.h"
#include "log_store.h"
#include "store.h"

static const char* STORE_TAG = "storage.c";

#define USER_FILE_PATH_FORMAT "/spiffs/user_%05lu.json"
#define USER_FILE_MAX_PATH 64
#define MAX_USER_COUNT 400

static bool spiffs_mounted = false;

static esp_err_t ensure_spiffs(void) {
    if (spiffs_mounted) {
        return ESP_OK;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 8,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(STORE_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(STORE_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(STORE_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Failed to get SPIFFS info (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(STORE_TAG, "SPIFFS mounted: total=%d, used=%d", total, used);
    }

    spiffs_mounted = true;
    return ESP_OK;
}

static esp_err_t set_char(const char* key, const char* value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(my_handle, key, value);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        ESP_LOGW(STORE_TAG, "NVS full while saving %s; clearing system logs and retrying", key);
        nvs_close(my_handle);
        log_store_clear();
        char log_msg[LOG_STORE_MESSAGE_MAX];
        snprintf(log_msg, sizeof(log_msg), "Cleared logs to free space for key %s", key);
        automation_record_log(log_msg);

                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err == ESP_OK) {
                    err = nvs_set_str(my_handle, key, value);
        }
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) setting string %s, continuing anyway", esp_err_to_name(err), key);
    }

    if (err == ESP_OK) {
        esp_err_t commit_err = nvs_commit(my_handle);
        if (commit_err != ESP_OK) {
            ESP_LOGW(STORE_TAG, "Error (%s) committing %s", esp_err_to_name(commit_err), key);
            err = commit_err;
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved %s", key);
    }
    } else {
        ESP_LOGE(STORE_TAG, "Failed to save %s: %s", key, esp_err_to_name(err));
    }

    nvs_close(my_handle);
    return err;
}

char* get_char(const char* key) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return strdup("");
    }

    size_t required_size = 0;
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(STORE_TAG, "[%s] value not initialized, key: %s", STORE_TAG, key);
        nvs_close(my_handle);
        return strdup("");
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) reading size for %s", esp_err_to_name(err), key);
        nvs_close(my_handle);
        return strdup("");
    }

    char* value = malloc(required_size);
    if (value == NULL) {
        ESP_LOGE(STORE_TAG, "Failed to allocate memory for %s", key);
        nvs_close(my_handle);
        return strdup("");
    }

    err = nvs_get_str(my_handle, key, value, &required_size);
    if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) reading %s", esp_err_to_name(err), key);
        free(value);
        nvs_close(my_handle);
        return strdup("");
    }

    nvs_close(my_handle);
    return value;
}

void set_bool(const char* key, bool value) {
    static SemaphoreHandle_t storage_mutex = NULL;
    
    // Initialize mutex on first call
    if (storage_mutex == NULL) {
        storage_mutex = xSemaphoreCreateMutex();
        if (storage_mutex == NULL) {
            ESP_LOGE(STORE_TAG, "Failed to create storage mutex!");
            return;
        }
    }
    
    // Wait for mutex with timeout
    if (xSemaphoreTake(storage_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(STORE_TAG, "Timeout waiting for storage mutex for key: %s", key);
        return;
    }
    
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        xSemaphoreGive(storage_mutex);
        return;
    }

    uint8_t val = value ? 1 : 0;
    bool retried = false;

    while (true) {
    err = nvs_set_u8(my_handle, key, val);
        if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE && !retried) {
            ESP_LOGW(STORE_TAG, "NVS full while saving bool %s; clearing system logs and retrying", key);
        nvs_close(my_handle);
            log_store_clear();
            char log_msg[LOG_STORE_MESSAGE_MAX];
            snprintf(log_msg, sizeof(log_msg), "Cleared logs to free space for key %s", key);
            automation_record_log(log_msg);
            retried = true;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
            if (err != ESP_OK) {
                break;
            }
            continue;
        }
        break;
    }

    if (err == ESP_OK) {
        esp_err_t commit_err = nvs_commit(my_handle);
        if (commit_err != ESP_OK) {
            ESP_LOGE(STORE_TAG, "Error (%s) committing bool %s", esp_err_to_name(commit_err), key);
            err = commit_err;
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved bool %s", key);
    }
    } else {
        ESP_LOGE(STORE_TAG, "Failed to save bool %s: %s", key, esp_err_to_name(err));
    }

    nvs_close(my_handle);
    xSemaphoreGive(storage_mutex);

    if (err != ESP_OK) {
        char msg[LOG_STORE_MESSAGE_MAX];
        snprintf(msg, sizeof(msg), "Failed to persist bool %s (err=%s)", key, esp_err_to_name(err));
        automation_record_log(msg);
    }
}

bool get_bool(const char* key, bool default_value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return default_value;
    }

    uint8_t value = default_value ? 1 : 0;
    err = nvs_get_u8(my_handle, key, &value);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(STORE_TAG, "Bool key %s not found, returning default", key);
        nvs_close(my_handle);
        return default_value;
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) reading bool %s, returning default", esp_err_to_name(err), key);
        nvs_close(my_handle);
        return default_value;
    }

    nvs_close(my_handle);
    return value != 0;
}

static esp_err_t set_u32(const char* key, uint32_t value) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    bool retried = false;
    while (true) {
    err = nvs_set_u32(my_handle, key, value);
        if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE && !retried) {
            ESP_LOGW(STORE_TAG, "NVS full while saving u32 %s; clearing system logs and retrying", key);
        nvs_close(my_handle);
            log_store_clear();
            char log_msg[LOG_STORE_MESSAGE_MAX];
            snprintf(log_msg, sizeof(log_msg), "Cleared logs to free space for key %s", key);
            automation_record_log(log_msg);
            retried = true;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
            if (err != ESP_OK) {
                break;
            }
            continue;
        }
        break;
    }

    if (err == ESP_OK) {
        esp_err_t commit_err = nvs_commit(my_handle);
        if (commit_err != ESP_OK) {
            ESP_LOGE(STORE_TAG, "Error (%s) committing u32 %s", esp_err_to_name(commit_err), key);
            err = commit_err;
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved u32 %s", key);
    }
    } else {
        ESP_LOGE(STORE_TAG, "Failed to save u32 %s: %s", key, esp_err_to_name(err));
    }

    nvs_close(my_handle);

    if (err != ESP_OK) {
        char msg[LOG_STORE_MESSAGE_MAX];
        snprintf(msg, sizeof(msg), "Failed to persist u32 %s (err=%s)", key, esp_err_to_name(err));
        automation_record_log(msg);
    }

    return err;
}

uint32_t get_u32(const char* key, uint32_t default_value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return default_value;
    }

    uint32_t value = default_value;
    err = nvs_get_u32(my_handle, key, &value);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(STORE_TAG, "[get_u32]: Error (%s) reading!", esp_err_to_name(err));
        nvs_close(my_handle);
        return default_value;
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) reading u32 %s", esp_err_to_name(err), key);
        nvs_close(my_handle);
        return default_value;
    }

    nvs_close(my_handle);
    return value;
}

// Wrapper functions for backward compatibility
esp_err_t store_char(const char* key, const char* value) {
    return set_char(key, value);
}

void store_u32(const char* key, uint32_t value) {
    esp_err_t err = set_u32(key, value);
    if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "store_u32 failed for %s (err=%s)", key, esp_err_to_name(err));
    }
}

// Dummy implementations for missing functions
esp_err_t initialize_spiffs(void) {
    return ensure_spiffs();
}

void load_wifi_credentials_from_flash(char *ssid, char *password) {
    char *ssid_str = get_char("wifi_ssid");
    char *password_str = get_char("wifi_password");

    if (strcmp(ssid_str, "")==0 || strcmp(password_str, "")==0) {
        strcpy(ssid, "");
        strcpy(password, "");
    } else {
        strcpy(ssid, ssid_str);
        strcpy(password, password_str);
    }

    free(ssid_str);
    free(password_str);
}

void load_server_info_from_flash(char *server_ip, char *server_port) {
    ESP_LOGI(STORE_TAG, "Loading Server info from flash");
    char *ip_str = get_char("server_ip");
    char *port_str = get_char("server_port");

    if (strcmp(ip_str, "")==0) {
        ESP_LOGI(STORE_TAG, "No server IP found in flash, setting to default.");
        strcpy(server_ip, "192.168.1.43");
    } else {
        strcpy(server_ip, ip_str);
    }
    
    if (strcmp(port_str, "")==0) {
        ESP_LOGI(STORE_TAG, "No server port found in flash, setting to default.");
        strcpy(server_port, "9000");
    } else {
        strcpy(server_port, port_str);
    }
    
    free(ip_str);
    free(port_str);
}

/* ---------- WiFi list management ---------- */
static uint64_t now_ms_store(void) { return esp_timer_get_time() / 1000ULL; }
static esp_err_t wifi_list_save_array(cJSON *arr); // forward

static cJSON *wifi_list_load_array(void) {
    char *json = get_char("wifi_list");
    cJSON *arr = NULL;
    if (json && json[0] != '\0') {
        arr = cJSON_Parse(json);
    }
    if (!cJSON_IsArray(arr)) {
        if (arr) cJSON_Delete(arr);
        arr = cJSON_CreateArray();
        /* Persist an empty list to avoid repeated NVS-not-found logs */
        if (arr) {
            wifi_list_save_array(arr);
        }
    }
    free(json);
    return arr;
}

static esp_err_t wifi_list_save_array(cJSON *arr) {
    if (!arr) return ESP_ERR_INVALID_ARG;
    char *json = cJSON_PrintUnformatted(arr);
    if (!json) return ESP_ERR_NO_MEM;
    esp_err_t err = store_char("wifi_list", json);
    free(json);
    return err;
}

static void wifi_list_remove_ssid(cJSON *arr, const char *ssid) {
    if (!arr || !ssid) return;
    int count = cJSON_GetArraySize(arr);
    for (int i = count - 1; i >= 0; i--) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        const cJSON *s = cJSON_GetObjectItemCaseSensitive(item, "ssid");
        if (cJSON_IsString(s) && s->valuestring && strcmp(s->valuestring, ssid) == 0) {
            cJSON_DeleteItemFromArray(arr, i);
        }
    }
}

esp_err_t wifi_list_add(const char *ssid, const char *password) {
    if (!ssid || ssid[0] == '\0') return ESP_ERR_INVALID_ARG;
    cJSON *arr = wifi_list_load_array();
    if (!arr) return ESP_ERR_NO_MEM;
    wifi_list_remove_ssid(arr, ssid);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "ssid", ssid);
    cJSON_AddStringToObject(obj, "password", password ? password : "");
    cJSON_AddNumberToObject(obj, "last_used_ms", (double)now_ms_store());
    cJSON_AddItemToArray(arr, obj);
    esp_err_t err = wifi_list_save_array(arr);
    cJSON_Delete(arr);
    // also set active creds
    if (err == ESP_OK) {
        store_wifi_credentials_to_flash(ssid, password ? password : "");
    }
    return err;
}

esp_err_t wifi_list_delete(const char *ssid) {
    if (!ssid) return ESP_ERR_INVALID_ARG;
    cJSON *arr = wifi_list_load_array();
    if (!arr) return ESP_ERR_NO_MEM;
    wifi_list_remove_ssid(arr, ssid);
    esp_err_t err = wifi_list_save_array(arr);
    cJSON_Delete(arr);
    return err;
}

esp_err_t wifi_list_set_active(const char *ssid) {
    if (!ssid) return ESP_ERR_INVALID_ARG;
    cJSON *arr = wifi_list_load_array();
    if (!arr) return ESP_ERR_NO_MEM;
    bool found = false;
    int count = cJSON_GetArraySize(arr);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        const cJSON *s = cJSON_GetObjectItemCaseSensitive(item, "ssid");
        const cJSON *p = cJSON_GetObjectItemCaseSensitive(item, "password");
        if (cJSON_IsString(s) && s->valuestring && strcmp(s->valuestring, ssid) == 0) {
            const char *pwd = (cJSON_IsString(p) && p->valuestring) ? p->valuestring : "";
            store_wifi_credentials_to_flash(ssid, pwd);
            cJSON_ReplaceItemInObject(item, "last_used_ms", cJSON_CreateNumber((double)now_ms_store()));
            found = true;
            break;
        }
    }
    esp_err_t err = found ? wifi_list_save_array(arr) : ESP_ERR_NOT_FOUND;
    cJSON_Delete(arr);
    return err;
}

cJSON *wifi_list_snapshot(void) {
    cJSON *arr = wifi_list_load_array();
    if (!arr) return NULL;
    // Strip passwords for snapshot
    int count = cJSON_GetArraySize(arr);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        cJSON_DeleteItemFromObject(item, "password");
    }
    return arr;
}

void get_md5_from_flash(char *md5_hash, size_t size) {
    char *stored_md5 = get_char("firmware_md5");
    if (stored_md5 && strlen(stored_md5) > 0) {
        strncpy(md5_hash, stored_md5, size - 1);
        md5_hash[size - 1] = '\0';
    } else {
        md5_hash[0] = '\0';
    }
    free(stored_md5);
}

cJSON* load_user_from_flash(uint32_t user_id) {
    if (user_id == 0 || user_id > MAX_USER_COUNT) {
        return NULL;
    }

    if (ensure_spiffs() != ESP_OK) {
        return NULL;
    }

    char file_path[USER_FILE_MAX_PATH];
    snprintf(file_path, sizeof(file_path), USER_FILE_PATH_FORMAT, (unsigned long)(user_id - 1));

    FILE *file = fopen(file_path, "r");
    if (!file) {
        ESP_LOGW(STORE_TAG, "User file %s not found", file_path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, length, file);
    fclose(file);
    buffer[read] = '\0';

    cJSON *user = cJSON_Parse(buffer);
    free(buffer);
    return user;
}

static esp_err_t write_user_to_file(size_t index, cJSON *user) {
    if (!user) return ESP_ERR_INVALID_ARG;
    if (ensure_spiffs() != ESP_OK) {
        return ESP_FAIL;
    }

    char file_path[USER_FILE_MAX_PATH];
    snprintf(file_path, sizeof(file_path), USER_FILE_PATH_FORMAT, (unsigned long)index);

    FILE *file = fopen(file_path, "w");
    if (!file) {
        ESP_LOGE(STORE_TAG, "Failed to open %s for writing", file_path);
        return ESP_FAIL;
    }

    char *json = cJSON_PrintUnformatted(user);
    if (!json) {
        fclose(file);
        return ESP_ERR_NO_MEM;
    }

    fprintf(file, "%s", json);
    fclose(file);
    free(json);
    return ESP_OK;
}

void store_user_to_flash(char *uuid, char *name, char *pin) {
    if (!uuid || !name || !pin) {
        ESP_LOGE(STORE_TAG, "Invalid user payload");
        return;
    }

    uint32_t user_index = get_u32("auth_user_count", 0);
    if (user_index >= MAX_USER_COUNT) {
        ESP_LOGE(STORE_TAG, "Reached maximum user count");
        return;
    }

    cJSON *user = cJSON_CreateObject();
    if (!user) {
        ESP_LOGE(STORE_TAG, "Failed to allocate user json");
        return;
    }

    cJSON_AddStringToObject(user, "uuid", uuid);
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "pin", pin);

    if (write_user_to_file(user_index, user) == ESP_OK) {
        store_u32("auth_user_count", user_index + 1);
        ESP_LOGI(STORE_TAG, "Stored user %s", uuid);
    }
    cJSON_Delete(user);
}

char* find_pin_in_flash(const char* pin) {
    if (!pin) return NULL;

    uint32_t user_count = get_u32("auth_user_count", 0);
    ESP_LOGI(STORE_TAG, "Total User Count: %" PRIu32, user_count);

    for (uint32_t i = 0; i < user_count; i++) {
        cJSON *user = load_user_from_flash(i + 1);
        if (!user) continue;

        cJSON *pin_json = cJSON_GetObjectItemCaseSensitive(user, "pin");
        if (cJSON_IsString(pin_json) && strcmp(pin_json->valuestring, pin) == 0) {
            cJSON *name_json = cJSON_GetObjectItemCaseSensitive(user, "name");
            char *name = NULL;
            if (cJSON_IsString(name_json) && name_json->valuestring) {
                name = strdup(name_json->valuestring);
            }
            cJSON_Delete(user);
            return name;
        }
        cJSON_Delete(user);
    }

    return NULL;
}

void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin) {
    if (!uuid) return;

    uint32_t user_count = get_u32("auth_user_count", 0);
    for (uint32_t i = 0; i < user_count; i++) {
        cJSON *user = load_user_from_flash(i + 1);
        if (!user) continue;

        cJSON *uuid_json = cJSON_GetObjectItemCaseSensitive(user, "uuid");
        if (cJSON_IsString(uuid_json) && strcmp(uuid_json->valuestring, uuid) == 0) {
            if (newName) {
                cJSON_ReplaceItemInObject(user, "name", cJSON_CreateString(newName));
            }
            if (newPin) {
                cJSON_ReplaceItemInObject(user, "pin", cJSON_CreateString(newPin));
            }
            write_user_to_file(i, user);
            cJSON_Delete(user);
            return;
        }
        cJSON_Delete(user);
    }
}

void delete_user_from_flash(const char *uuid_to_delete) {
    if (!uuid_to_delete) return;

    uint32_t user_count = get_u32("auth_user_count", 0);
    for (uint32_t i = 0; i < user_count; i++) {
        cJSON *user = load_user_from_flash(i + 1);
        if (!user) continue;

        cJSON *uuid_json = cJSON_GetObjectItemCaseSensitive(user, "uuid");
        bool match = cJSON_IsString(uuid_json) && strcmp(uuid_json->valuestring, uuid_to_delete) == 0;
        cJSON_Delete(user);

        if (!match) continue;

        for (uint32_t j = i; j < user_count - 1; j++) {
            char src_path[USER_FILE_MAX_PATH];
            char dest_path[USER_FILE_MAX_PATH];
            snprintf(src_path, sizeof(src_path), USER_FILE_PATH_FORMAT, (unsigned long)(j + 1));
            snprintf(dest_path, sizeof(dest_path), USER_FILE_PATH_FORMAT, (unsigned long)j);
            rename(src_path, dest_path);
        }

        char last_path[USER_FILE_MAX_PATH];
        snprintf(last_path, sizeof(last_path), USER_FILE_PATH_FORMAT, (unsigned long)(user_count - 1));
        remove(last_path);
        store_u32("auth_user_count", user_count - 1);
        ESP_LOGI(STORE_TAG, "Deleted user %s", uuid_to_delete);
        return;
    }
}
esp_err_t store_server_info_to_flash(const char *server_ip, const char *server_port) {
    esp_err_t err_ip = store_char("server_ip", server_ip);
    esp_err_t err_port = store_char("server_port", server_port);

    /* Keep tunnel host/port in sync with server settings */
    if (err_ip == ESP_OK) {
        store_char("tunnel_host", server_ip);
    }
    if (err_port == ESP_OK) {
        store_char("tunnel_port", server_port);
    }

    if (err_ip == ESP_OK && err_port == ESP_OK) {
        char log_msg[LOG_STORE_MESSAGE_MAX];
        snprintf(log_msg, sizeof(log_msg), "Server settings updated (IP=%s, port=%s)", server_ip, server_port);
        automation_record_log(log_msg);
        return ESP_OK;
    }

    if (err_ip != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Failed to store server IP: %s", esp_err_to_name(err_ip));
        automation_record_log("Failed to update server IP in NVS");
        return err_ip;
    }

    ESP_LOGE(STORE_TAG, "Failed to store server port: %s", esp_err_to_name(err_port));
    automation_record_log("Failed to update server port in NVS");
    return err_port;
}
esp_err_t store_wifi_credentials_to_flash(const char *ssid, const char *password) {
    esp_err_t err_ssid = store_char("wifi_ssid", ssid);
    esp_err_t err_pass = store_char("wifi_password", password);

    if (err_ssid == ESP_OK && err_pass == ESP_OK) {
        size_t pass_len = strlen(password);
        char mask_buf[LOG_STORE_MESSAGE_MAX];
        if (pass_len == 0) {
            snprintf(mask_buf, sizeof(mask_buf), "WiFi credentials updated (SSID=%s, password empty)", ssid);
        } else {
            size_t mask_len = pass_len < 32 ? pass_len : 32;
            char mask[33];
            memset(mask, '*', mask_len);
            mask[mask_len] = '\0';
            snprintf(mask_buf, sizeof(mask_buf), "WiFi credentials updated (SSID=%s, password=%s)", ssid, mask);
        }
        ESP_LOGI(STORE_TAG, "Stored WiFi credentials for SSID '%s' (password length=%zu)", ssid, pass_len);
        automation_record_log(mask_buf);
        return ESP_OK;
    }

    if (err_ssid != ESP_OK) {
        char msg[LOG_STORE_MESSAGE_MAX];
        snprintf(msg, sizeof(msg), "Failed to store WiFi SSID (err=%s)", esp_err_to_name(err_ssid));
        ESP_LOGE(STORE_TAG, "%s", msg);
        automation_record_log(msg);
        return err_ssid;
    }

    char msg[LOG_STORE_MESSAGE_MAX];
    snprintf(msg, sizeof(msg), "Failed to store WiFi password (err=%s)", esp_err_to_name(err_pass));
    ESP_LOGE(STORE_TAG, "%s", msg);
    automation_record_log(msg);
    return err_pass;
}

// Dummy implementations for missing setting functions
int storeSetting(char *key, cJSON *payload) {
    if (!payload) {
        return -1;
    }

    char *json = cJSON_PrintUnformatted(payload);
    if (!json) {
        cJSON_Delete(payload);
        return -1;
    }

    esp_err_t err = store_char(key, json);
    free(json);
    cJSON_Delete(payload);
    return err == ESP_OK ? 0 : -1;
}

int restoreSetting(char *key) {
    char *stored = get_char(key);
    if (strcmp(stored, "") != 0) {
        cJSON *msg = cJSON_Parse(stored);
        if (msg) {
            addServiceMessageToQueue(msg); // ownership transferred
        }
    }
    free(stored);
    return 0;
}
