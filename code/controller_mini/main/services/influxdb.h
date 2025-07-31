#ifndef INFLUXDB_H
#define INFLUXDB_H

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "cJSON.h"

// Define default IP and database name
#define INFLUXDB_IP "http://192.168.1.42:8086"  // Update with the correct IP and port
#define INFLUXDB_DB "thermostat"

// Function prototypes
void send_data_to_influxdb(float temperature, float humidity);
void send_json_to_influxdb(const char* measurement, cJSON* json);
static void get_mac_address(char *mac_str, size_t mac_str_size);

#endif // INFLUXDB_H