#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

char wss_data_in[1800];
char wss_data_out[1800];
bool wss_data_out_ready = false;

bool run_relay = true;
bool get_time = true;

char token[700];
char device_id[100];
bool disconnect_from_relay = false;
bool connect_to_relay = true;

static const char *SERVER_URI = CONFIG_SERVER_URI;

void
send_state(cJSON* state)
{
	char *state_str = cJSON_PrintUnformatted(state);
	snprintf(wss_data_out,sizeof(wss_data_out),""
	"{\"event_type\":\"load\","
	" \"payload\":{\"services\":["
	"{\"id\":\"light_1\", \"type\":\"light\","
	"\"state\":%s"
	"}]}}", state_str);
	free(state_str);
}

int
char_count(char * ch1, char * ch2, char* str) {
	int m;
	int charcount = 0;

	charcount = 0;
	for(m=0; str[m]; m++) {
	    if(str[m] == ch1) {
	        charcount ++;
	    }
			if(str[m] == ch2) {
					charcount --;
			}
	}
	return charcount;
}

int
check_json(char * str)
{
	int bra_cnt = char_count("{","}",str);
	if (bra_cnt!=0) return 0;
	return 1;
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
			start_time = current_time;
			cJSON *time_json = cJSON_CreateNumber(start_time);
			cJSON_ReplaceItemInObjectCaseSensitive(state,"start_time",time_json);
		}
	}
}

int
handle_event(char * event_type)
{

  // printf("looking for event type: %s\n",event_type);

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

	if (strcmp(event_type,"dimmer")==0) {
		// dimmer_payload = payload;
		// payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"light")==0) {
		led_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"grow-pod")==0) {
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
		current_time = cJSON_GetObjectItem(payload,"time")->valueint;
		// schedule_payload = payload;
		// payload = NULL;
		printf("Current Time: %lu\n", current_time);
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

	if (cJSON_GetObjectItem(root,"payload")) {
		payload = cJSON_GetObjectItemCaseSensitive(root,"payload");
	} else {
		ESP_LOGW(TAG, "payload key not found");
	}

	// Reply with callback
	if (cJSON_GetObjectItemCaseSensitive(root,"id")) {
		int callback_id = cJSON_GetObjectItemCaseSensitive(root,"id")->valueint;
		snprintf(callback,sizeof(callback),"{\"id\":%d,\"callback\":true,\"payload\":[false,\"\"]}",callback_id);
		strcpy(wss_data_out,callback);
		wss_data_out_ready = true;
	}

  if (cJSON_GetObjectItem(root,"event_type")) {
	  snprintf(event_type,sizeof(event_type),"%s",cJSON_GetObjectItem(root,"event_type")->valuestring);
	  return handle_event(event_type);
  }

  return handle_event(payload);
}

static void
websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    char rcv_buffer[1000];
    // esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");


            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;

        case WEBSOCKET_EVENT_DATA:
						if (data->data_len < 5) break;
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
            ESP_LOGW(TAG, "Received=%.*s\r\n", data->data_len, (char*)data->data_ptr);
            strcpy(rcv_buffer,(char*)data->data_ptr);

            int valid_json = check_json(rcv_buffer);
            cJSON *root = cJSON_Parse(rcv_buffer);

            if (root == NULL)
            {
                break;
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                }
                valid_json = 0;
            }

            if (!valid_json) {
              printf("invalid incoming json\n");
              break;
            }
            int res = ws_event_handler(root);
            if (res == 0) printf("event_type not found\n");
            if (res == -1) {
              printf("Reconnecting...\n");
							connect_to_relay = true;
            }

            break;
      case WEBSOCKET_EVENT_ERROR:
          ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
          break;
    }
}

static void
websocket_relay_task(void *pvParameter)
{
    char relay_uri[100];
    strcpy(relay_uri, SERVER_URI);
    strcat(relay_uri, "/device-relay");
		while (!connected_to_server) {
    	ESP_LOGI(TAG, "Waiting for IP...");
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
    ESP_LOGI(TAG, "Connecting to %s...", relay_uri);

    char headers[900];

		esp_websocket_client_handle_t client = NULL;

    while (1) {

			if (connect_to_relay) {
				connect_to_relay = false;
				snprintf(headers, sizeof(headers),
				"x-device-id: %s\r\n"
				"x-device-token: %s\r\n"
				"x-device-type: generic\r\n",
				device_id,
				token);

		    const esp_websocket_client_config_t websocket_cfg = {
		        .uri = relay_uri,
						.headers = headers,
		    };

		    client = esp_websocket_client_init(&websocket_cfg);
		    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
		    esp_websocket_client_start(client);
			}

			if (disconnect_from_relay) {
				esp_websocket_client_stop(client);
		    ESP_LOGI(TAG, "Websocket Stopped");
		    esp_websocket_client_destroy(client);
				connect_to_relay = true;
				disconnect_from_relay = false;
			}

      if (esp_websocket_client_is_connected(client) && strcmp(token,"")!=0) {
        if (strcmp(wss_data_out,"")!=0) {
          // int len = snprintf(wss_data_out,sizeof(wss_data_out),wss_data_out);
					char buf[2000];
					int len = sprintf(buf, "%s", wss_data_out);
          // ESP_LOGW(TAG, "Sending %s", wss_data_out);
          esp_websocket_client_send(client, buf, len, portMAX_DELAY);
					wss_data_out_ready = false;
					strcpy(wss_data_out,"");
        }
      }
      vTaskDelay(200 / portTICK_RATE_MS);
    }
}

static void
websocket_utilities_task(void *pvParameter)
{
    char utilities_uri[100];
    strcpy(utilities_uri, SERVER_URI);
    strcat(utilities_uri, "/utilities");
		while (!connected_to_server) {
			ESP_LOGI(TAG, "Waiting for IP...");
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
    ESP_LOGI(TAG, "Connecting to %s...", utilities_uri);

    const esp_websocket_client_config_t websocket_cfg = {
        .uri = utilities_uri,
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
    char data[64];
    int i = 0;
    while (1) {
      if (esp_websocket_client_is_connected(client)) {
        if (strcmp(device_id,"")==0) {
          int len = snprintf(data,sizeof(data),"{\"event_type\":\"generate-uuid\"}");
          ESP_LOGI(TAG, "Sending %s", data);
          esp_websocket_client_send(client, data, len, portMAX_DELAY);
        }
				if (get_time) {
					int len = snprintf(data,sizeof(data),"{\"event_type\":\"time\"}");
					ESP_LOGI(TAG, "Requesting time: %s", data);
					esp_websocket_client_send(client, data, len, portMAX_DELAY);
				}
      }
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(client);
}

void
websocket_main()
{
		if (cJSON_GetObjectItem(state,"start_time")) {
	    start_time = cJSON_GetObjectItem(state,"start_time")->valueint;
	    printf("loaded start_time: %lu\n", start_time);
	  }

	  if (cJSON_GetObjectItem(state,"time")) {
	    current_time = cJSON_GetObjectItem(state,"time")->valueint;
	    printf("loaded current time: %lu\n", current_time);
	  }

	  // tcpip_adapter_init();
	  // ESP_ERROR_CHECK(esp_event_loop_create_default());

	  strcpy(device_id,get_char("device_id"));
	  if (strcmp(device_id,"")==0) {
	    xTaskCreate(&websocket_utilities_task, "websocket_utilities_task", 10000, NULL, 5, NULL);
	  } else {
	    printf("pulled device_id from storage: %s\n", token);
	  }

	  strcpy(token,get_char("token"));
	  if (strcmp(token,"")==0) {
	    strcpy(token,device_id);
	    printf("no token found, setting token as device id: %s\n", token);
	  } else {
	    printf("pulled token from storage: %s\n", token);
	  }

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

	  xTaskCreate(&websocket_utilities_task, "websocket_utilities_task", 10000, NULL, 5, NULL);
	  xTaskCreate(&websocket_relay_task, "websocket_relay_task", 20000, NULL, 5, NULL);
}
