#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

int ws_event_handler(cJSON * root)
{
    char event_type[500];
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
            ESP_LOGW(TAG, "Payload is not a JSON object.");
        }
        
    } else {
        ESP_LOGW(TAG, "Payload key not found.");
    }

	return 0;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }
        break;

    case WEBSOCKET_EVENT_DATA:
        // ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        cJSON *root = cJSON_Parse(data->data_ptr);
        if (root) {
            ws_event_handler(root);
            cJSON_Delete(root);
            root = NULL;
        } else {
            if (strcmp(data->data_ptr, "") != 0) {
                ESP_LOGE(TAG, "JSON Parse Error");
            }
        }

        if (data->data_ptr) ((char *)data->data_ptr)[0] = '\0'; // Clear data
        break;

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }
        break;
    }
}

static void ws_client_task(void *pvParameters)
{
    char* server_info = (char*)pvParameters;
    char server_ip[32];
    char server_port[6];
    
    sscanf(server_info, "%31[^:]:%5s", server_ip, server_port);

    char uri[256];
    snprintf(uri, sizeof(uri), "ws://%s:%s/device-relay", server_ip, server_port);

	char headers[900];
	snprintf(headers, sizeof(headers),
	"x-device-id: %s\r\n"
	"x-device-token: %s\r\n"
	"x-device-type: generic\r\n",
	device_id,
	token);

	const esp_websocket_client_config_t websocket_cfg = {
		.uri = uri,
		.headers = headers,
	};

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    char data[32];
    int i = 0;
    while (1) {
        if (esp_websocket_client_is_connected(client)) {
            connect_to_relay = true;
			if (serverMessage.readyToSend) {
				int len = sprintf(data, serverMessage.message);
				char * data = serverMessage.message;
            	esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
				serverMessage.readyToSend = false;
				ESP_LOGI(TAG, "Sending server a message. (%d): %s\n", serverMessage.queueCount, serverMessage.message);
			}
        } else {
            connect_to_relay = false;
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }

    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
}


void ws_client_main(const char* ip, const char* port)
{
    printf("starting websocket client service\n");

    static char server_info[40];  // Static so it remains valid for the lifetime of the task
    snprintf(server_info, sizeof(server_info), "%s:%s", ip, port);

    xTaskCreate(&ws_client_task, "ws_client_task", 9 * 1000, server_info, 5, NULL);
}
