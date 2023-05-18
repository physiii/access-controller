/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


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
// #include <cJSON.h>

#define NO_DATA_TIMEOUT_SEC 5

char wss_data_in[1800];
char wss_data_out[1800];
bool wss_data_out_ready = false;

cJSON *payload = NULL;
bool run_relay = true;
bool get_time = true;

char token[700];
char device_id[100];
bool disconnect_from_relay = false;
bool connect_to_relay = true;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void
handle_settings(cJSON *settings)
{
	if (cJSON_GetObjectItem(settings,"calibrate_ph")) {
		if (cJSON_IsTrue(cJSON_GetObjectItem(settings,"calibrate_ph"))) {
			// calibrate_ph();
		}
	}

	if (cJSON_GetObjectItem(settings,"reset_cycletime")) {
		if (cJSON_IsTrue(cJSON_GetObjectItem(settings,"reset_cycletime"))) {
			// start_time = current_time;
			// cJSON *time_json = cJSON_CreateNumber(start_time);
			// cJSON_ReplaceItemInObjectCaseSensitive(state,"start_time",time_json);
		}
	}
}

int
handle_event(char * event_type)
{
	ESP_LOGI(WS_TAG, "handle_event: %s", cJSON_PrintUnformatted(payload));
	addServiceMessageToQueue(payload);

	if (strcmp(event_type,"settings")==0) {
		// dimmer_payload = payload;
		// payload = NULL;

		if (cJSON_GetObjectItem(payload,"settings")) {
			cJSON *settings = cJSON_GetObjectItem(payload,"settings");
			handle_settings(settings);
		}
		//
		// send_state();
		return 1;
	}

  // printf("looking for event type: %s\n",event_type);
	if (strcmp(event_type,"dimmer")==0) {
		// dimmer_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"access-control")==0) {
		// dimmer_payload = payload;
		// payload = NULL;

		if (cJSON_GetObjectItem(payload,"light_level")) {
			int level = cJSON_GetObjectItem(payload,"light_level")->valueint;
			// setSwitch(level);
		}
		//
		// send_state();
		return 1;
	}

	if (strcmp(event_type,"alarm")==0) {
		// alarm_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"motion")==0) {
		// motion_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"button")==0) {
		// button_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"microphone")==0) {
		// microphone_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"schedule")==0) {
		// schedule_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"load")==0) {
		char result[500];
		snprintf(result,sizeof(result),"%s",cJSON_GetObjectItem(payload,"result")->valuestring);
		printf("loaded: %s\n", result);
		return 1;
	}

	if (strcmp(event_type,"token")==0) {
		snprintf(token,sizeof(token),"%s",cJSON_GetObjectItem(payload,"token")->valuestring);
		printf("token received: %s\n", token);
		store_char("token",token);
		return 1;
	}

	if (strcmp(event_type,"reconnect-to-relay")==0) {
		printf("reconnecting to relay!\n");
		return -1;
	}

	if (strcmp(event_type,"authentication")==0) {
		char error[500];
		snprintf(error,sizeof(error),"%s",cJSON_GetObjectItem(payload,"error")->valuestring);
		printf("websocket: %s\n", error);
		return 1;
	}

	if (strcmp(event_type,"time")==0) {
		// current_time = cJSON_GetObjectItem(payload,"time")->valueint;
		// schedule_payload = payload;
		// payload = NULL;
		// printf("Current Time: %lu\n", current_time);
		get_time = false;
		return 1;
	}

	if (cJSON_GetObjectItem(payload,"uuid")) {
		snprintf(device_id,sizeof(device_id),"%s",cJSON_GetObjectItem(payload,"uuid")->valuestring);
		store_char("device_id",device_id);
		return 1;
	}
	return 0;
}

int
ws_event_handler(cJSON * root)
{
	char uuid[100];
	char event_type[500];
	char callback[70];

	// Reply with callback
	if (cJSON_GetObjectItemCaseSensitive(root,"id")) {
		int callback_id = cJSON_GetObjectItemCaseSensitive(root,"id")->valueint;
		snprintf(callback,sizeof(callback),"{\"id\":%d,\"callback\":true,\"payload\":[false,\"\"]}",callback_id);
		addServerMessageToQueue(callback);
        ESP_LOGI(TAG, "Sending callback.");
	}

	if (cJSON_GetObjectItem(root,"payload")) {
		payload = cJSON_GetObjectItemCaseSensitive(root,"payload");
	} else {
		ESP_LOGW(TAG, "Payload key not found.");
		return 0;
	}

	if (cJSON_GetObjectItem(root,"event_type")) {
		snprintf(event_type,sizeof(event_type),"%s",cJSON_GetObjectItem(root,"event_type")->valuestring);
		// return handle_event(event_type);
	} else {
		ESP_LOGW(TAG, "Event type key not found.");
		return 0;
	}

	return handle_event(payload);
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
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        // ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        // if (data->op_code == 0x08 && data->data_len == 2) {
        //     ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        // } else {
        //     ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        // }

        // If received data contains json structure it succeed to parse
        cJSON *root = cJSON_Parse(data->data_ptr);
        if (root) {
            int res = ws_event_handler(root);
            if (res == 0) printf("event_type not found\n");
            if (res == -1) {
        		printf("Reconnecting...\n");
				// connect_to_relay = true;
            }
        }
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

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

static void ws_client_task(void)
{
	char headers[900];
	snprintf(headers, sizeof(headers),
	"x-device-id: %s\r\n"
	"x-device-token: %s\r\n"
	"x-device-type: generic\r\n",
	device_id,
	token);

	const esp_websocket_client_config_t websocket_cfg = {
		.uri = "ws://192.168.1.42:5050/device-relay",
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
			// if (clientMessage.readyToSend) {
			// 	printf("Sending client a message (%d): %s\n", clientMessage.queueCount, clientMessage.message);
			// 	int len = sprintf(data, clientMessage.message);
			// 	char * data = clientMessage.message;
            // 	esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
			// 	clientMessage.readyToSend = false;
			// } else
			if (serverMessage.readyToSend) {
				ESP_LOGI(TAG, "Sending server a message. (%d): %s\n", serverMessage.queueCount, serverMessage.message);
				int len = sprintf(data, serverMessage.message);
				char * data = serverMessage.message;
            	esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
				serverMessage.readyToSend = false;
			}
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

static void ws_utilities_task(void)
{
	char headers[900];
	snprintf(headers, sizeof(headers),
	"x-device-id: %s\r\n"
	"x-device-token: %s\r\n"
	"x-device-type: generic\r\n",
	device_id,
	token);

	const esp_websocket_client_config_t websocket_cfg = {
		.uri = "ws://192.168.1.42:5050/utilities",
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
			if (strcmp(device_id,"")==0) {
			int len = snprintf(data,sizeof(data),"{\"event_type\":\"generate-uuid\"}");
			ESP_LOGI(TAG, "Sending %s", data);
			esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
			}
					if (get_time) {
						int len = snprintf(data,sizeof(data),"{\"event_type\":\"time\"}");
						ESP_LOGI(TAG, "Requesting time: %s", data);
						esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
					}
		}
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ws_client_main(void)
{
  	printf("starting websocket client service\n");

  	xTaskCreate(&ws_client_task, "ws_client_task", 5000, NULL, 5, NULL);
  	// xTaskCreate(&ws_utilities_task, "ws_client_task", 5000, NULL, 5, NULL);
}
