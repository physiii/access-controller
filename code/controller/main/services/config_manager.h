#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "esp_err.h"
#include "cJSON.h"

void generate_uuid_v4(char *uuid_str, size_t max_len);
void load_device_settings(char *device_id, size_t device_id_size, char *token, size_t token_size);
void get_mac_address(char *mac_str, size_t mac_str_size);
void get_device_id(char *device_id, size_t device_id_size);
void get_ip_address(char *ip_str, size_t ip_str_size);
void set_device_name(const char *device_name);
void get_device_name(char *device_name, size_t device_name_size);
void set_room_name(const char *room_name);
void get_room_name(char *room_name, size_t room_name_size);
void handle_config_manager_message(cJSON *message);
void config_manager_start();

#endif // CONFIG_MANAGER_H
