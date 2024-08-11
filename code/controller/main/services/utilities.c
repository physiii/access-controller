#include "utilities.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "automation.h"

static const char *UTIL_TAG = "Utilities";

// WebSocket event handler for ws_utilities_task
void utilities_websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(UTIL_TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(UTIL_TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(UTIL_TAG, "WEBSOCKET_EVENT_DATA received");
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(UTIL_TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

void ws_utilities_task(void *param) {
    char headers[900];
    snprintf(headers, sizeof(headers),
             "x-device-id: %s\r\n"
             "x-device-token: %s\r\n"
             "x-device-type: generic\r\n",
             device_id,
             token);

    char uri[500];
    snprintf(uri, sizeof(uri), "ws://%s:%s/utilities", server_ip, server_port);

    const esp_websocket_client_config_t websocket_cfg = {
        .uri = uri,
        .headers = headers,
    };

    ESP_LOGI(UTIL_TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, utilities_websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    char data[32];
    bool sent_uuid_request = false;

    while (1) {
        if (esp_websocket_client_is_connected(client)) {
            if (strcmp(device_id, "") == 0 && !sent_uuid_request) {
                sent_uuid_request = true;
                int len = snprintf(data, sizeof(data), "{\"event_type\":\"generate-uuid\"}");
                ESP_LOGI(UTIL_TAG, "Sending %s", data);
                esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
            }

            if (get_time) {
                int len = snprintf(data, sizeof(data), "{\"event_type\":\"time\"}");
                ESP_LOGI(UTIL_TAG, "Requesting time: %s", data);
                esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
            }
        }
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }

    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
}

esp_http_client_handle_t http_client_init(const char *url) {
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(UTIL_TAG, "Failed to initialize HTTP client");
    }
    return client;
}

esp_err_t http_perform_get_request(esp_http_client_handle_t client, char *output_buffer, int max_len, int *bytes_read) {
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(UTIL_TAG, "HTTP GET Status = %d, content length = %lld",
                 esp_http_client_get_status_code(client),
                 (long long int)esp_http_client_get_content_length(client));

        *bytes_read = esp_http_client_read(client, output_buffer, max_len);
        if (*bytes_read >= 0) {
            output_buffer[*bytes_read] = '\0';  // Null-terminate whatever we have read
        } else {
            ESP_LOGE(UTIL_TAG, "HTTP read error");
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(UTIL_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    return err;
}

void monitor_system_status() {
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

        // Log system status
        ESP_LOGI(TAG, "------ SYSTEM STATUS ------");
        ESP_LOGI(TAG, "Uptime: %d days %d hours %d minutes %d seconds", days, hours, minutes, seconds);
        ESP_LOGI(TAG, "Minimum Free Heap: %zu bytes", min_free_heap);
        ESP_LOGI(TAG, "NVS Free Entries: %u", nvs_stats.free_entries);
        ESP_LOGI(TAG, "NVS Used Entries: %u", nvs_stats.used_entries);
        ESP_LOGI(TAG, "----------------------------");

        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);

}
