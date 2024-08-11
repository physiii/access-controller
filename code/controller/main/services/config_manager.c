#include "config_manager.h"
#include "store.h"
#include "esp_log.h"
#include "automation.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "lwip/sockets.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

static const char *CONFIG_TAG = "ConfigManager";
static const char *DEFAULT_DEVICE_NAME = "ESP_Device";

void generate_uuid_v4(char *uuid_str, size_t max_len) {
    ESP_LOGI(CONFIG_TAG, "Generating UUID v4");
    unsigned char uuid[16];
    esp_fill_random(uuid, sizeof(uuid));

    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x3f) | 0x80;

    snprintf(uuid_str, max_len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
             uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    ESP_LOGI(CONFIG_TAG, "Generated UUID: %s", uuid_str);
}

void load_device_settings(char *device_id, size_t device_id_size, char *token, size_t token_size) {
    ESP_LOGI(CONFIG_TAG, "Loading device settings");
    if (load_string_from_store("device_id", device_id, device_id_size) != ESP_OK) {
        ESP_LOGE(CONFIG_TAG, "Failed to load Device ID, generating new one");
        char uuid_str[37];
        generate_uuid_v4(uuid_str, sizeof(uuid_str));
        strncpy(device_id, uuid_str, device_id_size);
        save_string_to_store("device_id", device_id);
        ESP_LOGI(CONFIG_TAG, "New Device ID generated and saved: %s", device_id);
    } else {
        ESP_LOGI(CONFIG_TAG, "Device ID loaded: %s", device_id);
    }

    if (load_string_from_store("token", token, token_size) != ESP_OK) {
        ESP_LOGI(CONFIG_TAG, "Failed to load token, using device id as token");
        strncpy(token, device_id, token_size);
        ESP_LOGI(CONFIG_TAG, "Token set to Device ID: %s", token);
    } else {
        ESP_LOGI(CONFIG_TAG, "Token loaded successfully");
    }
}

void get_mac_address(char *mac_str, size_t mac_str_size) {
    ESP_LOGI(CONFIG_TAG, "Getting MAC address");
    uint8_t mac[6];
    esp_err_t ret = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (ret == ESP_OK) {
        snprintf(mac_str, mac_str_size, "%02X%02X%02X%02X%02X%02X", 
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(CONFIG_TAG, "MAC address: %s", mac_str);
    } else {
        ESP_LOGE(CONFIG_TAG, "Failed to get MAC address. Error: %s", esp_err_to_name(ret));
    }
}

void get_device_id(char *device_id, size_t device_id_size) {
    ESP_LOGI(CONFIG_TAG, "Getting device ID");
    if (load_string_from_store("device_id", device_id, device_id_size) != ESP_OK) {
        ESP_LOGE(CONFIG_TAG, "Failed to load Device ID");
    } else {
        ESP_LOGI(CONFIG_TAG, "Device ID: %s", device_id);
    }
}

void get_ip_address(char *ip_str, size_t ip_str_size) {
    ESP_LOGI(CONFIG_TAG, "Getting IP address");
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            snprintf(ip_str, ip_str_size, IPSTR, IP2STR(&ip_info.ip));
            ESP_LOGI(CONFIG_TAG, "IP address: %s", ip_str);
        } else {
            ESP_LOGE(CONFIG_TAG, "Failed to get IP address");
        }
    } else {
        ESP_LOGE(CONFIG_TAG, "No network interface found");
    }
}

void set_device_name(const char *device_name) {
    ESP_LOGI(CONFIG_TAG, "Attempting to set device name to: %s", device_name);
    
    char current_name[50] = {0};
    esp_err_t ret = load_string_from_store("device_name", current_name, sizeof(current_name));
    
    if (ret == ESP_ERR_NOT_FOUND) {
        ESP_LOGI(CONFIG_TAG, "Device name not found, initializing with default");
        save_string_to_store("device_name", DEFAULT_DEVICE_NAME);
    }
    
    ret = save_string_to_store("device_name", device_name);
    if (ret != ESP_OK) {
        ESP_LOGE(CONFIG_TAG, "Failed to save Device Name. Error: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(CONFIG_TAG, "Device Name successfully set to: %s", device_name);
    }
}

void get_device_name(char *device_name, size_t device_name_size) {
    ESP_LOGI(CONFIG_TAG, "Attempting to get device name");
    esp_err_t ret = load_string_from_store("device_name", device_name, device_name_size);
    if (ret == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(CONFIG_TAG, "Device Name not found, initializing with default");
        strncpy(device_name, DEFAULT_DEVICE_NAME, device_name_size);
        save_string_to_store("device_name", DEFAULT_DEVICE_NAME);
    } else if (ret != ESP_OK) {
        ESP_LOGE(CONFIG_TAG, "Failed to load Device Name. Error: %s", esp_err_to_name(ret));
        strncpy(device_name, DEFAULT_DEVICE_NAME, device_name_size);
    } else {
        ESP_LOGI(CONFIG_TAG, "Successfully loaded device name: %s", device_name);
    }
}

void handle_set_device_name(cJSON *message) {
    cJSON *deviceNameItem = cJSON_GetObjectItem(message, "deviceName");
    if (!deviceNameItem || !cJSON_IsString(deviceNameItem)) {
        ESP_LOGE(CONFIG_TAG, "Invalid or missing deviceName in setDeviceName payload");
        return;
    }

    const char *new_name = deviceNameItem->valuestring;
    ESP_LOGI(CONFIG_TAG, "Setting device name to: %s", new_name);
    set_device_name(new_name);

    char verified_name[50];
    get_device_name(verified_name, sizeof(verified_name));

    char response[200];
    snprintf(response, sizeof(response), 
        "{\"event_type\":\"load\", \"payload\":{"
        "\"services\":[{\"id\":\"cm_1\", \"type\":\"config-manager\","
        "\"state\":{\"device_name\":\"%s\"}}]}}", 
        verified_name);
    ESP_LOGI(CONFIG_TAG, "Sending confirmation: %s", response);
    addServerMessageToQueue(response);
}

void handle_get_device_info() {
    char device_id[37];
    char mac_address[13];
    char ip_address[16];
    char device_name[50];

    get_device_id(device_id, sizeof(device_id));
    get_mac_address(mac_address, sizeof(mac_address));
    get_ip_address(ip_address, sizeof(ip_address));
    get_device_name(device_name, sizeof(device_name));

    char msg[500];
    snprintf(msg, sizeof(msg), 
        "{\"event_type\":\"load\", \"payload\":{"
        "\"services\":[{\"id\":\"cm_1\", \"type\":\"config-manager\","
        "\"state\":{"
        "\"device_id\":\"%s\","
        "\"mac_address\":\"%s\","
        "\"ip_address\":\"%s\","
        "\"device_name\":\"%s\""
        "}}]}}", 
        device_id, mac_address, ip_address, device_name);
    ESP_LOGI(CONFIG_TAG, "Sending device info: %s", msg);
    addServerMessageToQueue(msg);
}

static void config_manager_service(void *pvParameter) {
    ESP_LOGI(CONFIG_TAG, "Config manager service task started");
    while (1) {
        cJSON *message = checkServiceMessage("setDeviceName");
        if (message) {
            ESP_LOGI(CONFIG_TAG, "Received setDeviceName message");
            handle_set_device_name(message);
            cJSON_Delete(message);
        }
        message = checkServiceMessage("getDeviceInfo");
        if (message) {
            ESP_LOGI(CONFIG_TAG, "Received getDeviceInfo message");
            handle_get_device_info();
            cJSON_Delete(message);
        }
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void config_manager_start() {
    ESP_LOGI(CONFIG_TAG, "Starting config manager service");
    xTaskCreate(config_manager_service, "config_manager_service_task", 25 * 1000, NULL, 5, NULL);
    ESP_LOGI(CONFIG_TAG, "Config manager service task created");
}
