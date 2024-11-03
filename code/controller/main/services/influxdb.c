#include "influxdb.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "config_manager.h"

#define INFLUXDB_TAG "InfluxDB"

// Helper function to escape special characters in tag values
static void escape_tag_value(const char* input, char* output, size_t output_size) {
    size_t i, j;
    for (i = 0, j = 0; input[i] != '\0' && j < output_size - 1; i++) {
        if (input[i] == ' ' || input[i] == ',' || input[i] == '=') {
            output[j++] = '\\';
        }
        output[j++] = input[i];
    }
    output[j] = '\0';
}

void send_data_to_influxdb(float temperature, float humidity) {
    char post_data[512];
    char url[256];
    char device_id[37];
    char token[37];
    char mac_str[18];
    char device_name[32];
    char room_name[32];
    char escaped_device_name[64];

    load_device_settings(device_id, sizeof(device_id), token, sizeof(token));
    get_mac_address(mac_str, sizeof(mac_str));
    get_device_name(device_name, sizeof(device_name));
    get_room_name(room_name, sizeof(room_name));
    escape_tag_value(device_name, escaped_device_name, sizeof(escaped_device_name));

    snprintf(url, sizeof(url), "%s/write?db=%s", INFLUXDB_IP, INFLUXDB_DB);
    snprintf(post_data, sizeof(post_data), 
             "temperature,device_id=%s,mac=%s,device_name=%s,room_name=%s value=%.2f\n"
             "humidity,device_id=%s,mac=%s,device_name=%s,room_name=%s value=%.2f",
             device_id, mac_str, escaped_device_name, room_name, temperature,
             device_id, mac_str, escaped_device_name, room_name, humidity);

    esp_http_client_config_t config = {
        .url = url,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(INFLUXDB_TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(INFLUXDB_TAG, "Data sent successfully: %s", post_data);
    } else {
        ESP_LOGE(INFLUXDB_TAG, "Error sending data: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void send_json_to_influxdb(const char* measurement, cJSON* json) {
    char post_data[1024];
    char url[256];
    char field_data[512] = {0};
    int field_count = 0;
    char device_id[37];
    char token[37];
    char mac_str[18];
    char device_name[32];
    char escaped_device_name[64];

    load_device_settings(device_id, sizeof(device_id), token, sizeof(token));
    get_mac_address(mac_str, sizeof(mac_str));
    get_device_name(device_name, sizeof(device_name));
    escape_tag_value(device_name, escaped_device_name, sizeof(escaped_device_name));

    snprintf(url, sizeof(url), "%s/write?db=%s", INFLUXDB_IP, INFLUXDB_DB);

    cJSON *item = json->child;
    while (item) {
        if (cJSON_IsNumber(item)) {
            snprintf(field_data + strlen(field_data), sizeof(field_data) - strlen(field_data), 
                     "%s=%f,", item->string, item->valuedouble);
            field_count++;
        } else if (cJSON_IsBool(item)) {
            snprintf(field_data + strlen(field_data), sizeof(field_data) - strlen(field_data), 
                     "%s=%s,", item->string, cJSON_IsTrue(item) ? "true" : "false");
            field_count++;
        } else if (cJSON_IsString(item)) {
            snprintf(field_data + strlen(field_data), sizeof(field_data) - strlen(field_data), 
                     "%s=\"%s\",", item->string, item->valuestring);
            field_count++;
        }
        item = item->next;
    }

    // Remove trailing comma
    if (field_count > 0) {
        field_data[strlen(field_data) - 1] = '\0';
    }

    snprintf(post_data, sizeof(post_data), "%s,device_id=%s,mac=%s,device_name=%s %s",
             measurement, device_id, mac_str, escaped_device_name, field_data);

    esp_http_client_config_t config = {
        .url = url,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(INFLUXDB_TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(INFLUXDB_TAG, "Data sent successfully: %s", post_data);
    } else {
        ESP_LOGE(INFLUXDB_TAG, "Error sending data: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}