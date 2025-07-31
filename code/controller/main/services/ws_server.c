// ws_server.c
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "protocol_examples_common.h"
#include "freertos/semphr.h"

#include <esp_http_server.h>
#include "automation.h"

#define CONFIG_ESP_WIFI_CHANNEL 1 // Set your desired channel
#define CONFIG_ESP_MAX_STA_CONN 4 // Set your desired max STA connections
#define MAX_WS_CLIENTS 2

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

struct async_resp_arg ws_clients[MAX_WS_CLIENTS];
int active_clients = 0;
SemaphoreHandle_t ws_mutex = NULL;

static esp_err_t echo_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(WS_TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(WS_TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(WS_TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed with %d", ret);
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
			
            if (xSemaphoreTake(ws_mutex, portMAX_DELAY)) {
                // Add client to list if not already present
                int client_fd = httpd_req_to_sockfd(req);
                bool client_exists = false;
                for (int i = 0; i < active_clients; i++) {
                    if (ws_clients[i].fd == client_fd) {
                        client_exists = true;
                        break;
                    }
                }
                
                if (!client_exists && active_clients < MAX_WS_CLIENTS) {
                    ws_clients[active_clients].hd = req->handle;
                    ws_clients[active_clients].fd = client_fd;
                    active_clients++;
                    ESP_LOGI(WS_TAG, "Added new client, total clients: %d", active_clients);
                }
                xSemaphoreGive(ws_mutex);
            }
			
			return ESP_OK;
		} else {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) ESP_LOGE(WS_TAG, "Error before: %s\n", error_ptr);
		}

        ESP_LOGI(WS_TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(WS_TAG, "Packet type: %d", ws_pkt.type);

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(WS_TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}

static const httpd_uri_t echo = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static void ws_service (void *pvParameter)
{
  while (1) {
		if (clientMessage.readyToSend && active_clients > 0) {
			printf("Sending (%d): %s\n", clientMessage.queueCount, clientMessage.message);
			char * data = clientMessage.message;
            
            if (xSemaphoreTake(ws_mutex, portMAX_DELAY)) {
                // Send to all active clients
                for (int i = 0; i < active_clients; i++) {
                    httpd_handle_t hd = ws_clients[i].hd;
                    int fd = ws_clients[i].fd;
                    httpd_ws_frame_t ws_pkt;
                    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
                    ws_pkt.payload = (uint8_t*)data;
                    ws_pkt.len = strlen(data);
                    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

                    esp_err_t ret = httpd_ws_send_frame_async(hd, fd, &ws_pkt);
                    if (ret != ESP_OK) {
                        ESP_LOGE(WS_TAG, "Error sending to client %d: %d", i, ret);
                        // Remove disconnected client
                        for (int j = i; j < active_clients - 1; j++) {
                            ws_clients[j] = ws_clients[j + 1];
                        }
                        active_clients--;
                        i--; // Adjust index after removal
                    }
                }
                xSemaphoreGive(ws_mutex);
            }
			clientMessage.readyToSend = false;
            
            // Clear the message to free memory
            memset(clientMessage.message, 0, sizeof(clientMessage.message));
		}

        // Periodically check if clients are still connected
        static int cleanup_counter = 0;
        cleanup_counter++;
        if (cleanup_counter >= 100) { // Every 10 seconds
            if (xSemaphoreTake(ws_mutex, portMAX_DELAY)) {
                for (int i = 0; i < active_clients; i++) {
                    // Try to send a ping to check if client is alive
                    httpd_ws_frame_t ping_frame;
                    memset(&ping_frame, 0, sizeof(httpd_ws_frame_t));
                    ping_frame.type = HTTPD_WS_TYPE_PING;
                    ping_frame.len = 0;
                    
                    esp_err_t ret = httpd_ws_send_frame_async(ws_clients[i].hd, ws_clients[i].fd, &ping_frame);
                    if (ret != ESP_OK) {
                        ESP_LOGW(WS_TAG, "Client %d disconnected, removing", i);
                        // Remove disconnected client
                        for (int j = i; j < active_clients - 1; j++) {
                            ws_clients[j] = ws_clients[j + 1];
                        }
                        active_clients--;
                        i--; // Adjust index after removal
                    }
                }
                xSemaphoreGive(ws_mutex);
            }
            cleanup_counter = 0;
        }

		vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void start_ws_server(httpd_handle_t server)
{
	ESP_LOGI(WS_TAG, "Registering WS URI handlers");
	httpd_register_uri_handler(server, &echo);
	xTaskCreate(ws_service, "ws_service", 5000, NULL, 10, NULL);
}
