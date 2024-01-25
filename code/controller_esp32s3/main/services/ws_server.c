// ws_server.c
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "protocol_examples_common.h"

#include <esp_http_server.h>

#define CONFIG_ESP_WIFI_CHANNEL 1 // Set your desired channel
#define CONFIG_ESP_MAX_STA_CONN 4 // Set your desired max STA connections

bool should_send_data = 0; // wait for data to be received so hd an fd can be initialized

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

// Initialize resp_arg with a valid handle and invalid fd

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
    resp_arg = arg;
}


static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
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
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }

        cJSON *msg;
        msg = cJSON_Parse((char *)ws_pkt.payload);
		if (msg)
		{
			ESP_LOGI(WS_TAG, "Received: %s", cJSON_PrintUnformatted(msg));
            should_send_data = true;
			addServiceMessageToQueue(msg);
			return trigger_async_send(req->handle, req);
		} else {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) ESP_LOGE(WS_TAG, "Error before: %s\n", error_ptr);
		}

        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
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

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static void
ws_service (void *pvParameter)
{
  while (1) {
		if (clientMessage.readyToSend && should_send_data) {
			printf("Sending (%d): %s\n", clientMessage.queueCount, clientMessage.message);
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
	// resp_arg = malloc(sizeof(struct async_resp_arg));
}
