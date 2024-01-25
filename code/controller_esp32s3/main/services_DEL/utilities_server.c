typedef struct {
    char server_ip[32];
    char server_port[8];
} ServerInfo;

static void ws_utilities_task(void *param) {
    char headers[900];
        snprintf(headers, sizeof(headers),
        "x-device-id: %s\r\n"
        "x-device-token: %s\r\n"
        "x-device-type: generic\r\n",
        device_id,
        token);
    
    // Print server_ip and server_port
    printf("server_ip: %s\n", server_ip);
    
    char uri[500];
    snprintf(uri, sizeof(uri), "ws://%s:%s/utilities", server_ip, server_port);

    printf("uri: %s\n", uri);
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
    bool sent_uuid_request = false;

    while (1) {
        if (esp_websocket_client_is_connected(client)) {
            if (strcmp(device_id,"")==0 && !sent_uuid_request) {
                sent_uuid_request = true;
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
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

