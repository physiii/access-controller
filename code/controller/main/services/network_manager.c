#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"  // Include for event handling
#include "services/station.c"
#include "services/ap.c"
#include "automation.h"
#include "services/store.h"

static const char *NETWORK_MANAGER_TAG = "NetworkManager";
static bool is_connected = false;
static bool event_loop_created = false;

void network_manager_init(const char *device_id, const char *ssid, const char *password, const char *ap_ssid) {
    esp_err_t ret;

    // Initialize network interface first
    if (esp_netif_init() != ESP_OK) {
        ESP_LOGE(NETWORK_MANAGER_TAG, "Failed to initialize network interface");
        return;
    }

    // Create default event loop only if not already created
    if (!event_loop_created) {
        ret = esp_event_loop_create_default();
        if (ret != ESP_OK) {
            ESP_LOGE(NETWORK_MANAGER_TAG, "Failed to create event loop, error: %s", esp_err_to_name(ret));
            // return;
        }
        event_loop_created = true;
    }

    ESP_LOGI(NETWORK_MANAGER_TAG, "My SSID: %s", ssid);
    ESP_LOGI(NETWORK_MANAGER_TAG, "My Password: %s", password);

    // Initialize Wi-Fi in station mode if SSID and password are provided
    if (strlen(ssid) > 0 && strlen(password) > 0) {
        if (station_main(ssid, password)) {
            ESP_LOGI(NETWORK_MANAGER_TAG, "Connected in station mode");
            is_connected = true;
        } else {
            ESP_LOGI(NETWORK_MANAGER_TAG, "Failed to connect in station mode");
            is_connected = false;
        }
    }

    // Fallback to AP mode if not connected
    if (!is_connected) {
        ESP_LOGI(NETWORK_MANAGER_TAG, "Switching to AP mode");
        ap_main(device_id, ap_ssid);
    }
}

bool network_is_connected() {
    return is_connected;
}

void load_wifi_credentials(char *ssid, char *password, size_t ssid_len, size_t password_len) {
    load_string_from_store("wifiName", ssid, ssid_len);
    load_string_from_store("wifiPassword", password, password_len);
}
