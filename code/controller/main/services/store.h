#ifndef STORE_H
#define STORE_H

#include "cJSON.h"
#include <esp_err.h>
#include <stddef.h>
#include <stdint.h>

// Function declarations
void store_user_to_flash(char *uuid, char *name, char *pin);
cJSON* load_user_from_flash(uint32_t user_id);
void delete_user_from_flash(const char *uuid_to_delete);
void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin);
void store_char(const char *key, const char *value);
char* get_char(const char *key);
uint32_t get_u32(const char *key, uint32_t default_value);
void store_u32(const char *key, uint32_t value);
char* find_pin_in_flash(const char *pin);
void store_wifi_credentials_to_flash(const char *ssid, const char *password);
void load_wifi_credentials_from_flash(char *ssid, char *password);
void store_server_info_to_flash(const char *server_ip, const char *server_port);
void load_server_info_from_flash(char *server_ip, char *server_port);
char* get_md5_from_flash();
void store_md5_to_flash(const char *md5_hash);
int storeSetting(char *key, cJSON *payload);
cJSON* restoreSetting(char *key);
esp_err_t initialize_spiffs(void);
esp_err_t load_string_from_store(const char *key, char *value, size_t max_len);
esp_err_t save_string_to_store(const char *key, const char *value);
void addServiceMessageToQueue(cJSON *message);

#endif // STORE_H
