#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "automation.h"

static const char *WS_CLIENT = "WS_CLIENT";

// Struct to hold WebSocket client parameters
typedef struct {
    char server_ip[32];
    char server_port[6];
    char device_id[64];
    char token[600];
} ws_client_params_t;

int ws_event_handler(cJSON *root)
{
    char callback[70];

    // Reply with callback
    cJSON *idItem = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (idItem) {
        int callback_id = idItem->valueint;
        snprintf(callback, sizeof(callback), "{\"id\":%d,\"callback\":true,\"payload\":[false,\"\"]}", callback_id);
        addServerMessageToQueue(callback);
    }

    cJSON *payload = cJSON_GetObjectItemCaseSensitive(root, "payload");
    if (payload) {
        if (cJSON_IsObject(payload)) {
            addServiceMessageToQueue(payload);
        } else {
            ESP_LOGW(WS_CLIENT, "Payload is not a JSON object.");
        }
    } else {
        ESP_LOGW(WS_CLIENT, "Payload key not found.");
    }

    return 0;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(WS_CLIENT, "Last error %s: 0x%x", message, error_code);
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(WS_CLIENT, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(WS_CLIENT, "WEBSOCKET_EVENT_DISCONNECTED");
        log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
        }
        break;

    case WEBSOCKET_EVENT_DATA:
        cJSON *root = cJSON_Parse(data->data_ptr);
        if (root)
        {
            ws_event_handler(root);
            cJSON_Delete(root);
            root = NULL;
        }
        else
        {
            if (strcmp(data->data_ptr, "") != 0)
            {
                ESP_LOGE(WS_CLIENT, "JSON Parse Error");
            }
        }

        if (data->data_ptr)
            ((char *)data->data_ptr)[0] = '\0'; // Clear data
        break;

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(WS_CLIENT, "WEBSOCKET_EVENT_ERROR");
        log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
        }
        break;
    }
}

static void ws_client_task(void *pvParameters)
{
    ws_client_params_t *params = (ws_client_params_t *)pvParameters;

    char uri[256];
    snprintf(uri, sizeof(uri), "ws://%s:%s/device-relay", params->server_ip, params->server_port);

    char headers[900];
    snprintf(headers, sizeof(headers),
             "x-device-id: %s\r\n"
             "x-device-token: %s\r\n"
             "x-device-type: generic\r\n",
             params->device_id,
             params->token);

    const esp_websocket_client_config_t websocket_cfg = {
        .uri = uri,
        .headers = headers
    };

    ESP_LOGI(WS_CLIENT, "Headers: %s", headers);
    ESP_LOGI(WS_CLIENT, "Connecting to %s...", uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    char data[1024];

    while (1) {
        if (esp_websocket_client_is_connected(client)) {
            if (serverMessage.readyToSend) {
                int len = snprintf(data, sizeof(data), "%s", serverMessage.message);
                esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
                serverMessage.readyToSend = false;
                ESP_LOGI(WS_CLIENT, "Sending server a message. (%d): %s\n", serverMessage.queueCount, serverMessage.message);
            }
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }

    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
}

void ws_client_main(const char *ip, const char *port, const char *device_id, const char *token) {
    printf("starting websocket client service\n");

    static ws_client_params_t ws_params;
    strncpy(ws_params.server_ip, ip, sizeof(ws_params.server_ip) - 1);
    strncpy(ws_params.server_port, port, sizeof(ws_params.server_port) - 1);
    strncpy(ws_params.device_id, device_id, sizeof(ws_params.device_id) - 1);
    strncpy(ws_params.token, token, sizeof(ws_params.token) - 1);

    ESP_LOGI(WS_CLIENT, "Server Info: %s:%s", ws_params.server_ip, ws_params.server_port);
    ESP_LOGI(WS_CLIENT, "Device ID: %s", ws_params.device_id);
    ESP_LOGI(WS_CLIENT, "Token: %s", ws_params.token);

    xTaskCreate(&ws_client_task, "ws_client_task", 9 * 1000, &ws_params, 5, NULL);
}
