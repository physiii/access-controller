#include "automation.h"
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
#include "services/radar.c"
#include "services/keypad.c"
#include "services/fob.c"
#include "services/server.c"
#include "services/ap.c"
#include "services/ws_client.c"
#include "services/utilities_server.c"

void generate_ssid_from_device_id(char *device_id, char *ssid, size_t size) {
    if (strlen(device_id) >= 5) {
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


void app_main(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    strcpy(device_id, get_char("device_id"));
    if (strcmp(device_id, "") == 0) {
        ESP_LOGI(TAG, "No Device ID found, fetching UUID...");
        xTaskCreate(&ws_utilities_task, "ws_utilities_task", 10000, NULL, 5, NULL);
    } else {
        ESP_LOGI(TAG, "Device ID : %s", device_id);
    }

    strcpy(token, get_char("token"));
    if (strcmp(token, "") == 0) {
        strcpy(token, device_id);
        ESP_LOGI(TAG, "No token found, setting as device id");
    } else {
        ESP_LOGI(TAG, "Token: %s", token);
    }

    serviceMessage.read = true;
    serviceMessage.message = NULL;

    clientMessage.readyToSend = false;
    serverMessage.readyToSend = false;

    char wifi_ssid[32];
    char wifi_password[64];
    snprintf(wifi_ssid, sizeof(wifi_ssid), "pyfitech");
    snprintf(wifi_password, sizeof(wifi_password), "pyfitech");

    load_wifi_credentials_from_flash(wifi_ssid, wifi_password);
    if (!station_main(wifi_ssid, wifi_password)) {
        ESP_LOGI(TAG, "Starting Access Point...");
        
        char ap_ssid[32];
        generate_ssid_from_device_id(device_id, ap_ssid, sizeof(ap_ssid));
        
        ap_main(ap_ssid, "pyfitech");
    }

    gpio_main();
    i2c_main();
    mcp23x17_main();
    auth_main();
    lock_main();
    buzzer_main();
    wiegand_main();

    // exit_main();
    // keypad_main();
    #if STRIKE
        radar_main();
    #else
        // fob_main();
    #endif
    server_main();

    char server_ip[32];
    char server_port[8];
    load_server_info_from_flash(server_ip, server_port);
    
    ServerInfo serverInfo;
    strcpy(serverInfo.server_ip, server_ip);
    strcpy(serverInfo.server_port, server_port);
    xTaskCreate(&ws_utilities_task, "ws_utilities_task", 10000, &serverInfo, 5, NULL);
    
    ws_client_main(server_ip, server_port);

    if (initialize_spiffs() == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS Initialized successfully");
    }
    send_user_count();

    TaskHandle_t serviceMessageTaskHandle = NULL;
    TaskHandle_t clientMessageTaskHandle = NULL;
    TaskHandle_t serverMessageTaskHandle = NULL;

    xTaskCreate(serviceMessageTask, "serviceMessageTask", 5000, NULL, 10, &serviceMessageTaskHandle);
    xTaskCreate(clientMessageTask, "clientMessageTask", 5000, NULL, 10, &clientMessageTaskHandle);
    xTaskCreate(serverMessageTask, "serverMessageTask", 5000, NULL, 10, &serverMessageTaskHandle);

    int cnt = 0;
    TaskStatus_t *pxTaskStatusArray;
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

        // Get Task Status
        UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
        pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
        if (pxTaskStatusArray != NULL) {
            uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);
        }

        // Log system status
        ESP_LOGI(TAG, "------ SYSTEM STATUS ------");
        ESP_LOGI(TAG, "Uptime: %d days %d hours %d minutes %d seconds", days, hours, minutes, seconds);
        ESP_LOGI(TAG, "Minimum Free Heap: %zu bytes", min_free_heap);
        ESP_LOGI(TAG, "NVS Free Entries: %u", nvs_stats.free_entries);
        ESP_LOGI(TAG, "NVS Used Entries: %u", nvs_stats.used_entries);
        ESP_LOGI(TAG, "Loop Count: %d", cnt++);
        // ESP_LOGI(TAG, "--- Task Status ---");
        // for (UBaseType_t i = 0; i < uxArraySize; i++) {
        //     ESP_LOGI(TAG, "Task [%s] - State: %c Priority: %u", pxTaskStatusArray[i].pcTaskName,
        //             pxTaskStatusArray[i].eCurrentState, pxTaskStatusArray[i].uxCurrentPriority);
        // }
        ESP_LOGI(TAG, "----------------------------");

        // Cleanup
        vPortFree(pxTaskStatusArray);

        // Delay for 10 seconds
        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }
}
