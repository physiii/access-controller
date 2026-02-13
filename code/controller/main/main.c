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
#include "services/wiegand_registry.c"
#include "services/authorize.c"
#include "services/buzzer.c"
#include "services/lock.c"
#include "services/wiegand.c"
#include "services/exit.c"
#include "services/motion.c"
#include "services/keypad.c"
#include "services/fob.c"
#include "services/rf_registry.c"
#include "services/rf_receiver.c"
#include "services/server.c"
#include "services/tunnel.c"
#include "services/ap.c"
#include "services/api.c"
#include "esp_http_client.h"
#include "esp_random.h"
#include "esp_heap_caps.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

static void generate_uuid_v4(char *uuid, size_t size) {
    if (!uuid || size < 37) {
        if (uuid && size > 0) {
            uuid[0] = '\0';
        }
        return;
    }

    uint8_t bytes[16];
    for (int i = 0; i < 16; i += 4) {
        uint32_t value = esp_random();
        bytes[i] = (value >> 24) & 0xFF;
        bytes[i + 1] = (value >> 16) & 0xFF;
        bytes[i + 2] = (value >> 8) & 0xFF;
        bytes[i + 3] = value & 0xFF;
    }

    bytes[6] = (bytes[6] & 0x0F) | 0x40; // Version 4
    bytes[8] = (bytes[8] & 0x3F) | 0x80; // RFC 4122 variant

    snprintf(uuid, size,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             bytes[0], bytes[1], bytes[2], bytes[3],
             bytes[4], bytes[5],
             bytes[6], bytes[7],
             bytes[8], bytes[9],
             bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
}

static void ensure_device_identity(void) {
    char *stored_device_id = get_char("device_id");
    bool need_new_device_id = true;
    if (stored_device_id && stored_device_id[0] != '\0') {
        size_t len = strlen(stored_device_id);
        need_new_device_id = len < 8; // basic sanity check
    }

    if (need_new_device_id) {
        generate_uuid_v4(device_id, sizeof(device_id));
        if (store_char("device_id", device_id) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to persist device UUID");
        }
        ESP_LOGI(TAG, "Generated new device UUID: %s", device_id);
    } else {
        strncpy(device_id, stored_device_id, sizeof(device_id) - 1);
        device_id[sizeof(device_id) - 1] = '\0';
    }

    if (stored_device_id) {
        free(stored_device_id);
    }

    char *stored_token = get_char("token");
    if (stored_token && stored_token[0] != '\0') {
        strncpy(token, stored_token, sizeof(token) - 1);
        token[sizeof(token) - 1] = '\0';
    } else {
        strncpy(token, device_id, sizeof(token) - 1);
        token[sizeof(token) - 1] = '\0';
        if (store_char("token", token) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to persist token");
        }
        ESP_LOGI(TAG, "Token not found; defaulting to device UUID");
    }

    if (stored_token) {
        free(stored_token);
    }
}

void generate_ssid_from_device_id(char *device_id, char *ssid, size_t size) {
    char suffix[5] = "uuid";
    if (device_id && device_id[0] != '\0') {
        int count = 0;
        for (int i = (int)strlen(device_id) - 1; i >= 0 && count < 4; --i) {
            if (device_id[i] == '-') {
                continue;
            }
            suffix[3 - count] = (char)tolower((unsigned char)device_id[i]);
            count++;
        }
    }

    snprintf(ssid, size, "ac_%s", suffix);
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
    automation_log_boot_event();

    ensure_device_identity();
    ESP_LOGI(TAG, "Device UUID: %s", device_id);
    
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

        tunnel_start();
    } else {
        ESP_LOGI(TAG, "Starting Access Point...");
        automation_record_log("WiFi STA failed after retries; starting AP mode");
        
        char ap_ssid[32];
        generate_ssid_from_device_id(device_id, ap_ssid, sizeof(ap_ssid));
        
        ap_main(ap_ssid, "pyfitech");
    }

    gpio_main();
    i2c_main();
    mcp23x17_main();
    auth_main();
    buzzer_main();
    wiegand_registry_init();
    wiegand_main();
    exit_main();
    motion_main();
    keypad_main();
    fob_main();
    rf_registry_init();
    rf_receiver_init();
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

        size_t current_free_heap = esp_get_free_heap_size();
        size_t min_free_heap = esp_get_minimum_free_heap_size();
        size_t largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

        // Get NVS stats for the 'nvs' partition
        nvs_stats_t nvs_stats;
        esp_err_t err = nvs_get_stats("nvs", &nvs_stats);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get NVS stats (%s)", esp_err_to_name(err));
        }

        // Log system status
        ESP_LOGI(TAG, "------ SYSTEM STATUS ------");
        ESP_LOGI(TAG, "Uptime: %d days %d hours %d minutes %d seconds", days, hours, minutes, seconds);
        ESP_LOGI(TAG, "Free Heap: %zu bytes (min: %zu, largest block: %zu)", current_free_heap, min_free_heap, largest_free_block);
        ESP_LOGI(TAG, "NVS Free Entries: %u", nvs_stats.free_entries);
        ESP_LOGI(TAG, "NVS Used Entries: %u", nvs_stats.used_entries);
        ESP_LOGI(TAG, "Loop Count: %d", cnt++);
        ESP_LOGI(TAG, "----------------------------");

        vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
    }
}
