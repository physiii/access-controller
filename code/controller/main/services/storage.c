#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

char tag[50] = "[storage.c]";
bool storage_in_use = false;

void storage_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

char *get_char(const char *key) {
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Error (%d) opening NVS handle\n", err);
        return "";
    }

    size_t required_size;
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    char *value_str = malloc(required_size);
    nvs_get_str(my_handle, key, value_str, &required_size);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(tag, "%s value not initialized, key: %s\n", tag, key);
        free(value_str);
        nvs_close(my_handle);
        return "";
    } else if (err != ESP_OK) {
        ESP_LOGE(tag, "%s Error (%d) reading %s\n", tag, err, key);
        free(value_str);
        nvs_close(my_handle);
        return "";
    }

    nvs_close(my_handle);
    return value_str;
}

void store_char(const char *key, const char *value) {
    storage_in_use = true;
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Error (%d) opening NVS handle\n", err);
    } else {
        err = nvs_set_str(my_handle, key, value);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "%s nvs_set_str for %s failed!\n", tag, key);
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "%s nvs_commit for %s failed!\n", tag, key);
        }
        nvs_close(my_handle);
    }
    storage_in_use = false;
}

void store_u32(const char *key, uint32_t value) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Error (%d) opening NVS handle\n", err);
    } else {
        err = nvs_set_u32(my_handle, key, value);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Error (%d) writing!\n", err);
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Error (%d) committing!\n", err);
        }
        nvs_close(my_handle);
    }
}

uint32_t get_u32(const char *key, uint32_t default_value) {
    char tag[50] = "[get_u32]";
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle my_handle;
    uint32_t value = default_value;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Error (%d) opening NVS handle\n", err);
    } else {
        err = nvs_get_u32(my_handle, key, &value);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(tag, "Error (%d) reading!\n", err);
        } else if (err != ESP_OK) {
            ESP_LOGE(tag, "Error (%d) reading!\n", err);
        }
        nvs_close(my_handle);
    }
    return value;
}
