// main.c

#include "esp_https_ota.h"
#include "esp_event.h"
#include "automation.h"
#include "automation.c"
#include "services/station.c"
#include "services/drivers/i2c.c"
#include "services/drivers/mcp23x17.c"
#include "services/gpio.c"
#include "services/store.c"
#include "services/authorize.c"
#include "services/buzzer.c"
#include "services/lock.c"
#include "services/wiegand.c"
#include "services/exit.c"
#include "services/motion.c"
#include "services/keypad.c"
#include "services/fob.c"
#include "services/server.c"
#include "services/ap.c"
#include "esp_http_client.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include <time.h>
#include "cJSON.h"
#include "esp_timer.h"

char stored_firmware_md5[33];
bool need_to_update_firmware = true;

void generate_ssid_from_device_id(char *device_id, char *ssid, size_t size) {
    if (device_id && strlen(device_id) >= 5) {
        // Use the last 5 characters from device_id
        snprintf(ssid, size, "ac_%s", device_id + strlen(device_id) - 5);
    } else if (strcmp(device_id, "") == 0) {
        // If there's no device_id, use "ac_uuid"
        snprintf(ssid, size, "ac_uuid");
    } else {
        // If device_id length is less than 5 but not empty, use what's available
        snprintf(ssid, size, "ac_%s", device_id);
    }
}

void perform_ota_update(const char *ota_url) {
    esp_http_client_config_t http_config = {
        .url = ota_url,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
}

esp_err_t http_event_handle(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (strcmp((char*)evt->data, stored_firmware_md5) != 0) {
                need_to_update_firmware = true;
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void fetch_firmware_md5_from_server(char *buffer, size_t buffer_size, const char *server_ip, const char *server_port) {
    esp_http_client_config_t http_config = {
        .url = NULL,
    };

    char url[256];
    snprintf(url, sizeof(url), "http://%s:%s/firmware-md5", server_ip, server_port);
    http_config.url = url;

    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d", esp_http_client_get_status_code(client));
        
        int read_len = esp_http_client_read(client, buffer, buffer_size - 1);
        if (read_len <= 0) {
            // Handle the error, maybe set buffer to an empty string or a known value.
            buffer[0] = '\0';
            ESP_LOGE(TAG, "Failed to read response or response is empty");
        } else {
            buffer[read_len] = '\0'; // Null-terminate the buffer
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
}


void app_main(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    init_automation_queues();

    // Get device ID
    strcpy(device_id, get_char("device_id"));

    strcpy(token, get_char("token"));
    if (strcmp(token, "") == 0) {
        strcpy(token, device_id);
        ESP_LOGI(TAG, "No token found, setting as device id");
    } else {
        ESP_LOGI(TAG, "Token: %s", token);
    }
    
    char wifi_ssid[32];
    char wifi_password[64];
    snprintf(wifi_ssid, sizeof(wifi_ssid), "pyfitech");
    snprintf(wifi_password, sizeof(wifi_password), "pyfitech");

    load_wifi_credentials_from_flash(wifi_ssid, wifi_password);
    
    // Check if we have valid credentials before attempting station mode
    bool has_valid_credentials = (strlen(wifi_ssid) > 0 && strlen(wifi_password) > 0);
    ESP_LOGI(TAG, "WiFi credentials check: SSID='%s' (len=%d), Password='%s' (len=%d)", 
             wifi_ssid, (int)strlen(wifi_ssid), wifi_password, (int)strlen(wifi_password));
    
    if (has_valid_credentials && station_main(wifi_ssid, wifi_password)) {
        ESP_LOGI(TAG, "Successfully connected to WiFi in station mode");
        load_server_info_from_flash(server_ip, server_port);
        char ota_url[256];

        get_md5_from_flash(stored_firmware_md5, sizeof(stored_firmware_md5));
        // Fetch the latest firmware MD5 hash from the server
        char latest_firmware_md5[33];
        fetch_firmware_md5_from_server(latest_firmware_md5, sizeof(latest_firmware_md5), server_ip, server_port);

        snprintf(ota_url, sizeof(ota_url), "http://%s:%s/firmware.bin", server_ip, server_port);
        // Compare the latest firmware MD5 hash with the stored one
        need_to_update_firmware = strcmp(stored_firmware_md5, latest_firmware_md5) != 0;

        if (need_to_update_firmware) {
            perform_ota_update(ota_url);
        }

        if (strcmp(device_id, "") == 0) {
            ESP_LOGE(TAG, "Device ID not found");
        }
    } else {
        ESP_LOGI(TAG, "Starting Access Point...");
        
        char ap_ssid[32];
        generate_ssid_from_device_id(device_id, ap_ssid, sizeof(ap_ssid));
        
        ap_main(ap_ssid, "pyfitech");
    }

    gpio_main();
    i2c_main();
    mcp23x17_main();
    auth_main();
    buzzer_main();
    wiegand_main();
    exit_main();
    motion_main();
    keypad_main();
    fob_main();
    lock_main();
    server_main();

    if (initialize_spiffs() == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS Initialized successfully");
    }
    send_user_count();

    int cnt = 0;
    while (1) {
        // Get system information
        int64_t uptime_us = esp_timer_get_time();
        int64_t uptime_s = uptime_us / 1000000;
        int days = uptime_s / (24 * 3600);
        uptime_s %= (24 * 3600);
        int hours = uptime_s / 3600;
        uptime_s %= 3600;
        int minutes = uptime_s / 60;
        int seconds = uptime_s % 60;

        size_t min_free_heap = esp_get_minimum_free_heap_size();

        // Get NVS stats for the 'nvs' partition
        nvs_stats_t nvs_stats;
        esp_err_t err = nvs_get_stats("nvs", &nvs_stats);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get NVS stats (%s)", esp_err_to_name(err));
        }

        // Log system status
        ESP_LOGI(TAG, "------ SYSTEM STATUS ------");
        ESP_LOGI(TAG, "Uptime: %d days %d hours %d minutes %d seconds", days, hours, minutes, seconds);
        ESP_LOGI(TAG, "Minimum Free Heap: %zu bytes", min_free_heap);
        ESP_LOGI(TAG, "NVS Free Entries: %u", nvs_stats.free_entries);
        ESP_LOGI(TAG, "NVS Used Entries: %u", nvs_stats.used_entries);
        ESP_LOGI(TAG, "Loop Count: %d", cnt++);
        ESP_LOGI(TAG, "----------------------------");

        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }
}
