#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include "cJSON.h"
#include "esp_websocket_client.h"

// Function declarations
int ws_event_handler(cJSON *root);
void ws_client_main(const char* ip, const char* port);

// Internal function declarations
static void log_error_if_nonzero(const char *message, int error_code);
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void ws_client_task(void *pvParameters);

#endif // WS_CLIENT_H
