#ifndef UTILITIES_H
#define UTILITIES_H

#include "esp_http_client.h"
#include "esp_err.h"

// External variables
extern char token[700];
extern char device_id[100];
extern char server_ip[32];
extern char server_port[8];
extern bool get_time;
extern const char *TAG;

// Struct to hold server information
typedef struct {
    char server_ip[32];
    char server_port[8];
} ServerInfo;

// WebSocket event handler for ws_utilities_task
void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

// Task to manage WebSocket utilities
void ws_utilities_task(void *param);

// Initialize an HTTP client
esp_http_client_handle_t http_client_init(const char *url);

// Perform an HTTP GET request
esp_err_t http_perform_get_request(esp_http_client_handle_t client, char *output_buffer, int max_len, int *bytes_read);

// Helper function to monitor system status
void monitor_system_status();

#endif // UTILITIES_H
