/* WebSocket Echo Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"

#include <esp_http_server.h>

#include <cJSON.h>

// struct ServiceMessage
// {
// 	cJSON *message;
// 	bool read;
// };
//
// struct ServiceMessage *serviceMessage;

/* A simple example that demonstrates using websocket echo server
 */
static const char *WS_TAG = "ws_echo_server";

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

struct async_resp_arg *resp_arg;

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
	  resp_arg = arg;
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t echo_handler(httpd_req_t *req)
{
    uint8_t buf[128] = { 0 };
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
		cJSON *msg;
    msg = cJSON_Parse((char *)ws_pkt.payload);
		if (msg)
		{
			addServiceMessageToQueue(msg);
			ESP_LOGI(WS_TAG, "Received: %s", cJSON_PrintUnformatted(msg));
			return trigger_async_send(req->handle, req);
		} else {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) ESP_LOGE(WS_TAG, "Error before: %s\n", error_ptr);
		}

    return ret;
}

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

		config.server_port = 8080;
    // Start the httpd server
    ESP_LOGI(WS_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(WS_TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(WS_TAG, "Error starting server!");
    return NULL;
}

static void
ws_service (void *pvParameter)
{
  while (1) {
		if (clientMessage.readyToSend) {
			printf("Sending: %s\n", clientMessage.message);
			char * data = clientMessage.message;
			httpd_handle_t hd = resp_arg->hd;
			int fd = resp_arg->fd;
			httpd_ws_frame_t ws_pkt;
			memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
			ws_pkt.payload = (uint8_t*)data;
			ws_pkt.len = strlen(data);
			ws_pkt.type = HTTPD_WS_TYPE_TEXT;

			httpd_ws_send_frame_async(hd, fd, &ws_pkt);
			clientMessage.readyToSend = false;
		}

    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void start_ws_server(httpd_handle_t server)
{
	ESP_LOGI(WS_TAG, "Registering WS URI handlers");
	httpd_register_uri_handler(server, &ws);
	xTaskCreate(ws_service, "ws_service", 5000, NULL, 10, NULL);
	resp_arg = malloc(sizeof(struct async_resp_arg));
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(WS_TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(WS_TAG, "Starting webserver");
        *server = start_webserver();
    }
}
