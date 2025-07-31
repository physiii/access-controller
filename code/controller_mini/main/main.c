// main.c

#include "esp_event.h"
#include "nvs_flash.h"
#include "automation.h"
#include "cJSON.h"
#include "services/network_manager.c"  // Import network functionality
#include "services/ota_manager.c"      // Import OTA update functionality
#include "services/service_manager.c"  // Import service management
#include "services/config_manager.c"   // Import configuration management
#include "services/store.c"
#include "services/ws_client.c"
#include "services/utilities.c"

const char *TAG = "open-automation";

void app_main(void) {
    // Load device and network configurations
    char device_id[64] = {0};
    char token[600] = {0};
    char wifi_ssid[32] = {0};
    char wifi_password[64] = {0};
    char server_ip[64] = {0};
    char server_port[6] = {0};

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    load_device_settings(device_id, sizeof(device_id), token, sizeof(token));
    ESP_LOGI(TAG, "Device ID after load: %s", device_id);
    ESP_LOGI(TAG, "Token after load: %s", token);

    load_wifi_credentials(wifi_ssid, wifi_password, sizeof(wifi_ssid), sizeof(wifi_password));
    load_server_info_from_flash(server_ip, server_port);

    automationInit();

    // Network setup and decision making
    network_manager_init(device_id, wifi_ssid, wifi_password, "pyfitech");

    if (network_is_connected()) {
        ota_manager_check_update();  // Check for firmware updates
        service_manager_start_all(); // Start all peripheral services
        ws_client_main(server_ip, server_port, device_id, token);  // WebSocket and other server interactions
    } else {
        ap_main(device_id, "pyfitech"); // Handle AP mode if not connected to a station
        ota_manager_check_update();     // Check for firmware updates
        service_manager_start_all();    // Start all peripheral services
    }

    ESP_LOGI(TAG, "Monitoring system status...");
    while (1) {
        monitor_system_status();
        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }
}
