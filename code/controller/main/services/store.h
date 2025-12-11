#ifndef STORE_H
#define STORE_H

#include <stdint.h>
#include <stddef.h>
#include "cJSON.h"
#include "esp_err.h"

void store_user_to_flash(char *uuid, char *name, char *pin);
cJSON *load_user_from_flash(uint32_t user_id);
void delete_user_from_flash(const char *uuid_to_delete);
void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin);
esp_err_t store_char(const char *key, const char *value);
char *get_char(const char *key);
uint32_t get_u32(const char *key, uint32_t default_value);
void store_u32(const char *key, uint32_t value);
char *find_pin_in_flash(const char *pin);
esp_err_t store_wifi_credentials_to_flash(const char *ssid, const char *password);
void load_wifi_credentials_from_flash(char *ssid, char *password);
esp_err_t store_server_info_to_flash(const char *server_ip, const char *server_port);
void load_server_info_from_flash(char *server_ip, char *server_port);
esp_err_t wifi_list_add(const char *ssid, const char *password);
esp_err_t wifi_list_delete(const char *ssid);
cJSON *wifi_list_snapshot(void);
esp_err_t wifi_list_set_active(const char *ssid);
void get_md5_from_flash(char *md5_hash, size_t size);
esp_err_t initialize_spiffs(void);
int storeSetting(char *key, cJSON *payload);
int restoreSetting(char *key);

#endif // STORE_H

