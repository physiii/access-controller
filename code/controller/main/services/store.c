#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "automation.h"

static const char* STORE_TAG = "storage.c";

void set_char(const char* key, const char* value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(my_handle, key, value);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        ESP_LOGW(STORE_TAG, "NVS partition full, erasing and retrying...");
        nvs_close(my_handle);
        
        // Erase and reinitialize NVS
        esp_err_t ret = nvs_flash_erase();
        if (ret == ESP_OK) {
            ret = nvs_flash_init();
            if (ret == ESP_OK) {
                // Retry the operation
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err == ESP_OK) {
                    err = nvs_set_str(my_handle, key, value);
                    if (err == ESP_OK) {
                        ESP_LOGI(STORE_TAG, "Successfully saved %s after NVS cleanup", key);
                    } else {
                        ESP_LOGE(STORE_TAG, "Failed to save %s even after cleanup: %s", key, esp_err_to_name(err));
                    }
                }
            }
        }
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) setting string %s, continuing anyway", esp_err_to_name(err), key);
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved %s", key);
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) committing %s", esp_err_to_name(err), key);
    }

    nvs_close(my_handle);
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
    err = nvs_set_u8(my_handle, key, val);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        ESP_LOGW(STORE_TAG, "NVS partition full, erasing and retrying...");
        nvs_close(my_handle);
        
        // Erase and reinitialize NVS
        esp_err_t ret = nvs_flash_erase();
        if (ret == ESP_OK) {
            ret = nvs_flash_init();
            if (ret == ESP_OK) {
                // Retry the operation
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err == ESP_OK) {
                    err = nvs_set_u8(my_handle, key, val);
                    if (err == ESP_OK) {
                        ESP_LOGI(STORE_TAG, "Successfully saved bool %s after NVS cleanup", key);
                    } else {
                        ESP_LOGE(STORE_TAG, "Failed to save bool %s even after cleanup: %s", key, esp_err_to_name(err));
                    }
                }
            }
        }
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) setting bool %s, continuing anyway", esp_err_to_name(err), key);
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved bool %s", key);
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) committing bool %s", esp_err_to_name(err), key);
    }

    nvs_close(my_handle);
    xSemaphoreGive(storage_mutex);
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

void set_u32(const char* key, uint32_t value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(STORE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u32(my_handle, key, value);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        ESP_LOGW(STORE_TAG, "NVS partition full, erasing and retrying...");
        nvs_close(my_handle);
        
        // Erase and reinitialize NVS
        esp_err_t ret = nvs_flash_erase();
        if (ret == ESP_OK) {
            ret = nvs_flash_init();
            if (ret == ESP_OK) {
                // Retry the operation
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err == ESP_OK) {
                    err = nvs_set_u32(my_handle, key, value);
                    if (err == ESP_OK) {
                        ESP_LOGI(STORE_TAG, "Successfully saved u32 %s after NVS cleanup", key);
                    } else {
                        ESP_LOGE(STORE_TAG, "Failed to save u32 %s even after cleanup: %s", key, esp_err_to_name(err));
                    }
                }
            }
        }
    } else if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) setting u32 %s, continuing anyway", esp_err_to_name(err), key);
    } else {
        ESP_LOGI(STORE_TAG, "Successfully saved u32 %s", key);
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(STORE_TAG, "Error (%s) committing u32 %s", esp_err_to_name(err), key);
    }

    nvs_close(my_handle);
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
void store_char(const char* key, const char* value) {
    set_char(key, value);
}

void store_u32(const char* key, uint32_t value) {
    set_u32(key, value);
}

// Dummy implementations for missing functions
esp_err_t initialize_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
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
        ESP_LOGE(STORE_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(STORE_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ESP_OK;
}

// Dummy user management functions
void load_wifi_credentials_from_flash(char *ssid, char *password) {
    ESP_LOGI(STORE_TAG, "Loading WiFi credentials from flash");
    char *ssid_str = get_char("wifi_ssid");
    char *password_str = get_char("wifi_password");

    if (strcmp(ssid_str, "")==0 || strcmp(password_str, "")==0) {
        ESP_LOGI(STORE_TAG, "No WiFi credentials found in flash");
        strcpy(ssid, "");
        strcpy(password, "");
    } else {
        ESP_LOGI(STORE_TAG, "WiFi credentials found in flash: %s, %s.", ssid_str, password_str);
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

// Dummy implementations for missing user functions
cJSON* load_user_from_flash(uint32_t user_id) { return NULL; }
void store_user_to_flash(char *uuid, char *name, char *pin) {}
char* find_pin_in_flash(const char* pin) { return NULL; }
void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin) {}
void delete_user_from_flash(const char *uuid_to_delete) {}
void store_server_info_to_flash(const char *server_ip, const char *server_port) {
    store_char("server_ip", server_ip);
    store_char("server_port", server_port);
}
void store_wifi_credentials_to_flash(const char *ssid, const char *password) {
    store_char("wifi_ssid", ssid);
    store_char("wifi_password", password);
}

// Dummy implementations for missing setting functions
int storeSetting(char *key, cJSON *payload) {
    store_char(key, cJSON_PrintUnformatted(payload));
    return 0;
}

int restoreSetting(char *key) {
    char *stored = get_char(key);
    if (strcmp(stored, "") != 0) {
        cJSON *msg = cJSON_Parse(stored);
        if (msg) addServiceMessageToQueue(msg);
    }
    free(stored);
    return 0;
}
