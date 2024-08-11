#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include <string.h>  // For string functions like strcmp

// Unique TAG for this module
static const char *OTA_TAG = "OTAManager";

// Firmware and version information
typedef struct {
    char stored_firmware_md5[33];
    bool need_to_update_firmware;
} firmware_state_t;

static firmware_state_t firmware_state;

// Function to fetch firmware MD5 from the server
static esp_err_t fetch_firmware_md5_from_server(char *buffer, size_t buffer_size, const char *server_ip, const char *server_port) {
    char url[256];
    snprintf(url, sizeof(url), "http://%s:%s/firmware-md5", server_ip, server_port);

    esp_http_client_config_t http_config = {
        .url = url,
    };
    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(OTA_TAG, "HTTPS Status = %d", esp_http_client_get_status_code(client));
        int read_len = esp_http_client_read(client, buffer, buffer_size - 1);
        if (read_len > 0) {
            buffer[read_len] = '\0';
        } else {
            buffer[0] = '\0';
            ESP_LOGE(OTA_TAG, "Failed to read response or response is empty");
        }
    } else {
        ESP_LOGE(OTA_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

// Function to perform OTA update
static void perform_ota_update(const char *ota_url) {
    esp_https_ota_config_t ota_config = {
        .http_config = &(esp_http_client_config_t){ .url = ota_url }
    };

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(OTA_TAG, "Firmware update successful");
        esp_restart();
    } else {
        ESP_LOGE(OTA_TAG, "Firmware upgrade failed with error: %s", esp_err_to_name(ret));
    }
}

// Public function to check and initiate OTA update
void ota_manager_check_update() {
    // Placeholder for server details, should be replaced with actual retrieval logic
    char server_ip[64] = "192.168.1.100";
    char server_port[6] = "8080";
    char latest_firmware_md5[33];

    if (fetch_firmware_md5_from_server(latest_firmware_md5, sizeof(latest_firmware_md5), server_ip, server_port) == ESP_OK) {
        if (strcmp(firmware_state.stored_firmware_md5, latest_firmware_md5) != 0) {
            strcpy(firmware_state.stored_firmware_md5, latest_firmware_md5);
            firmware_state.need_to_update_firmware = true;
        }

        if (firmware_state.need_to_update_firmware) {
            char ota_url[256];
            snprintf(ota_url, sizeof(ota_url), "http://%s:%s/firmware.bin", server_ip, server_port);
            perform_ota_update(ota_url);
        }
    }
}

void ota_manager_init() {
    // Initialize firmware state
    memset(&firmware_state, 0, sizeof(firmware_state));
    // Example: Load the MD5 from non-volatile storage
    strcpy(firmware_state.stored_firmware_md5, "initial_md5_hash_value");
    firmware_state.need_to_update_firmware = false;

    ESP_LOGI(OTA_TAG, "OTA Manager Initialized with MD5: %s", firmware_state.stored_firmware_md5);
}
