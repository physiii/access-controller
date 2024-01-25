#include "storage.c"
#include <inttypes.h>
#include "esp_spiffs.h"
#include "esp_system.h"

#define USERS_PER_INDEX   8
#define MAX_USER_COUNT    10 * 1000
#define FILE_PATH_MAX_LENGTH 100

char store_service_message[1000];
bool store_service_message_ready = false;

struct Setting {
    char str[1000];
    uint32_t cnt;
    char key[50];
    cJSON *obj;
};

struct Setting setting;

typedef struct {
    char uuid[40];
    char name[100];
    char pin[10];
} UserData;

int storeSetting(char *key, cJSON *payload)
{
    store_char(key, cJSON_PrintUnformatted(payload));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    return 0;
}

int restoreSetting (char *key) {
	strcpy(setting.str, get_char(key));
	if (strcmp(setting.str, "")==0) return 1;

	cJSON *msg;
	msg = cJSON_Parse(setting.str);
	if (msg) addServiceMessageToQueue(msg);

	return 0;
}

esp_err_t initialize_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    // Check if SPIFFS was mounted
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    // Print SPIFFS size info (optional)
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ESP_OK;
}

void store_user_to_flash(char *uuid, char *name, char *pin) {
    ESP_LOGI(TAG, "Storing user to flash: UUID=%s, Name=%s", uuid, name);

    size_t user_index = get_u32("auth_user_count", 0);
    if (user_index >= MAX_USER_COUNT) {
        ESP_LOGE(TAG, "Reached maximum user count");
        return;
    }

    char file_path[FILE_PATH_MAX_LENGTH];
    snprintf(file_path, sizeof(file_path), "/spiffs/user_%zu.json", user_index);

    FILE* file = fopen(file_path, "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open user data file for writing.");
        return;
    }

    cJSON *user_json = cJSON_CreateObject();
    cJSON_AddStringToObject(user_json, "uuid", uuid);
    cJSON_AddStringToObject(user_json, "name", name);
    cJSON_AddStringToObject(user_json, "pin", pin);

    fprintf(file, "%s", cJSON_PrintUnformatted(user_json));

    cJSON_Delete(user_json);
    fclose(file);

    store_u32("auth_user_count", user_index + 1);
}

cJSON* load_user_from_flash(uint32_t user_id) {
    user_id -= 1; // Convert to 0-based index
    if (user_id >= MAX_USER_COUNT) return NULL;

    char file_path[FILE_PATH_MAX_LENGTH];
    snprintf(file_path, sizeof(file_path), "/spiffs/user_%lu.json", user_id);

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open user data file for reading.");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory.");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    cJSON *user_json = cJSON_Parse(buffer);
    free(buffer);
    fclose(file);

    return user_json;
}

void delete_user_from_flash(const char *uuid_to_delete) {
    ESP_LOGI(TAG, "Deleting user from flash: UUID=%s", uuid_to_delete);
    size_t user_count = get_u32("auth_user_count", 0);

    for (size_t i = 0; i < user_count; i++) {
        cJSON* user = load_user_from_flash(i + 1);
        if (user) {
            cJSON* uuid_json = cJSON_GetObjectItem(user, "uuid");
            if (uuid_json && (strcmp(uuid_json->valuestring, uuid_to_delete) == 0)) {
                char file_path[FILE_PATH_MAX_LENGTH];
                snprintf(file_path, sizeof(file_path), "/spiffs/user_%zu.json", i);
                remove(file_path);

                // Move remaining files to fill the gap
                for (size_t j = i; j < user_count - 1; j++) {
                    char src_path[FILE_PATH_MAX_LENGTH];
                    char dest_path[FILE_PATH_MAX_LENGTH];
                    snprintf(src_path, sizeof(src_path), "/spiffs/user_%zu.json", j + 1);
                    snprintf(dest_path, sizeof(dest_path), "/spiffs/user_%zu.json", j);
                    rename(src_path, dest_path);
                }

                store_u32("auth_user_count", user_count - 1);
                cJSON_Delete(user);
                return;
            }
            cJSON_Delete(user);
        }
    }

    ESP_LOGE(TAG, "User with UUID=%s not found.", uuid_to_delete);
}

void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin) {
    ESP_LOGI(TAG, "Modifying user in flash: UUID=%s", uuid);
    size_t user_count = get_u32("auth_user_count", 0);

    for (size_t i = 0; i < user_count; i++) {
        cJSON* user = load_user_from_flash(i + 1);
        if (user) {
            cJSON* uuid_json = cJSON_GetObjectItem(user, "uuid");
            if (uuid_json && (strcmp(uuid_json->valuestring, uuid) == 0)) {
                cJSON_ReplaceItemInObject(user, "name", cJSON_CreateString(newName));
                cJSON_ReplaceItemInObject(user, "pin", cJSON_CreateString(newPin));

                char file_path[FILE_PATH_MAX_LENGTH];
                snprintf(file_path, sizeof(file_path), "/spiffs/user_%zu.json", i);

                FILE* file = fopen(file_path, "w");
                if (file == NULL) {
                    ESP_LOGE(TAG, "Failed to open user data file for writing.");
                    cJSON_Delete(user);
                    return;
                }

                fprintf(file, "%s", cJSON_PrintUnformatted(user));
                fclose(file);
                cJSON_Delete(user);
                return;
            }
            cJSON_Delete(user);
        }
    }

    ESP_LOGE(TAG, "User with UUID=%s not found.", uuid);
}

char* find_pin_in_flash(const char* pin) 
{
    uint32_t user_count = get_u32("auth_user_count", 0);
    ESP_LOGI(TAG, "Total User Count: %" PRIu32, user_count);

    for (uint32_t i = 0; i < user_count; i++) { 
        cJSON *user = load_user_from_flash(i + 1); // Convert to 1-based index when loading user
        if (!user) continue;

        cJSON *pin_json = cJSON_GetObjectItem(user, "pin");
        if (pin_json && (strcmp(pin_json->valuestring, pin) == 0)) {
            char *uuid = strdup(cJSON_GetObjectItem(user, "uuid")->valuestring);
            char *name = strdup(cJSON_GetObjectItem(user, "name")->valuestring);
            cJSON_Delete(user);
            free(uuid);
            return name;
        }
        cJSON_Delete(user);
    }
    return NULL;
}

void store_wifi_credentials_to_flash(const char *ssid, const char *password) 
{
    store_char("wifi_ssid", ssid);
    store_char("wifi_password", password);
    ESP_LOGI(TAG, "wifi credentials stored: %s, %s", ssid, password);
}

void load_wifi_credentials_from_flash(char *ssid, char *password) 
{
    ESP_LOGI(TAG, "Loading WiFi credentials from flash");
	//init ssid and password and check if get_char returns null
	char *ssid_str = get_char("wifi_ssid");
	char *password_str = get_char("wifi_password");

	if (strcmp(ssid_str, "")==0 || strcmp(password_str, "")==0) {
		ESP_LOGI(TAG, "No WiFi credentials found in flash");
		return;
	}
	
	ESP_LOGI(TAG, "WiFi credentials found in flash: %s, %s.", ssid_str, password_str);
	strcpy(ssid, get_char("wifi_ssid"));
	strcpy(password, get_char("wifi_password"));
}

void store_server_info_to_flash(const char *server_ip, const char *server_port) 
{
    store_char("server_ip", server_ip);
    store_char("server_port", server_port);
    ESP_LOGI(TAG, "Server info stored: IP: %s, Port: %s", server_ip, server_port);
}

void load_server_info_from_flash(char *server_ip, char *server_port) 
{
    ESP_LOGI(TAG, "Loading Server info from flash");
    strcpy(server_ip, get_char("server_ip"));
    strcpy(server_port, get_char("server_port"));

    if (strcmp(server_ip, "")==0) {
        ESP_LOGI(TAG, "No server IP found in flash, setting to default.");
        strcpy(server_ip, "192.168.1.42");
    }
    if (strcmp(server_port, "")==0) {
        ESP_LOGI(TAG, "No server port found in flash, setting to default.");
        strcpy(server_port, "80");
    }
}

char* get_md5_from_flash() {
    return get_char("firmware_md5");
}

void store_md5_to_flash(const char *md5_hash) {
    store_char("firmware_md5", md5_hash);
    ESP_LOGI(TAG, "Stored MD5 hash to flash: %s", md5_hash);
}
