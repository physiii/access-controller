/*
 * Serve OTA update portal and other static content
 */
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "ws_server.c"
#include "file_server.c"

#define SERVER_TAG "SERVER"

void server_main()
{
	ESP_LOGI(SERVER_TAG, "Starting HTTP Server");
    /* Start the file server which will in turn start the ws server */
    if (start_file_server("/spiffs") != ESP_OK) {
        ESP_LOGE(SERVER_TAG, "Failed to start file server!");
    }
}
