#include "storage.c"
#include <inttypes.h>

#define USERS_PER_INDEX   8
#define MAX_USER_COUNT 1000

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

void store_user_to_flash(char *uuid, char *name, char *pin) {
    ESP_LOGI(TAG, "Storing user to flash: UUID=%s, Name=%s", uuid, name);   
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS with error: %d", err);
        return;
    }

    size_t user_index = get_u32("auth_user_count", 0);
    if (err != ESP_OK && err == ESP_ERR_NVS_NOT_FOUND) {
        user_index = 0;
        store_u32("auth_user_count", user_index);
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get user count with error: %d", err);
        nvs_close(my_handle);
        return;
    }

    if (user_index >= MAX_USER_COUNT) {
        ESP_LOGE(TAG, "Reached maximum user count");
        nvs_close(my_handle);
        return;
    }

    UserData user_group[USERS_PER_INDEX] = {0}; // Initialize the array
    char key[100];
    size_t start_index = (user_index / USERS_PER_INDEX) * USERS_PER_INDEX;

    snprintf(key, sizeof(key), "users_data_%zu", start_index / USERS_PER_INDEX);

    size_t expected_size = sizeof(user_group);
    err = nvs_get_blob(my_handle, key, user_group, &expected_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to read users data with error: %d", err);
        nvs_close(my_handle);
        return;
    }

    
    // Modify the correct user in the user_group
    UserData *user = &user_group[user_index % USERS_PER_INDEX];
    // print user
    strncpy(user->uuid, uuid, sizeof(user->uuid));
    strncpy(user->name, name, sizeof(user->name));
    strncpy(user->pin, pin, sizeof(user->pin));

    // Store the updated user_group back to NVS
    err = nvs_set_blob(my_handle, key, user_group, sizeof(user_group));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to store users data with error: %d", err);
    }
    
    // Update user count
    store_u32("auth_user_count", user_index + 1);

    nvs_commit(my_handle);
    nvs_close(my_handle);
}

cJSON* load_user_from_flash(uint32_t user_id) {
    user_id -= 1; // Convert to 0-based index
    if (user_id >= MAX_USER_COUNT) return NULL;

    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS with error: %d", err);
        return NULL;
    }

    // Allocate user_group on the heap instead of the stack
    UserData* user_group = malloc(USERS_PER_INDEX * sizeof(UserData));
    if (user_group == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for user group.");
        nvs_close(my_handle);
        return NULL;
    }

    char key[100];
    size_t start_index = (user_id / USERS_PER_INDEX) * USERS_PER_INDEX;

    snprintf(key, sizeof(key), "users_data_%zu", start_index / USERS_PER_INDEX);

    size_t expected_size = sizeof(UserData) * USERS_PER_INDEX;
    err = nvs_get_blob(my_handle, key, user_group, &expected_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read users data with error: %d", err);
        free(user_group);  // Free the allocated memory
        nvs_close(my_handle);
        return NULL;
    }

    UserData *user = &user_group[user_id % USERS_PER_INDEX];

    // Convert the user structure to cJSON and return
    cJSON *user_json = cJSON_CreateObject();
    cJSON_AddStringToObject(user_json, "uuid", user->uuid);
    cJSON_AddStringToObject(user_json, "name", user->name);
    cJSON_AddStringToObject(user_json, "pin", user->pin);

    free(user_group);  // Free the allocated memory
    nvs_close(my_handle);

    return user_json;
}

void delete_user_from_flash(const char *uuid) {
    ESP_LOGI(TAG, "Deleting user from flash: UUID=%s", uuid);
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS with error: %d", err);
        return;
    }

    size_t user_count = get_u32("auth_user_count", 0);
    bool user_found = false;

    UserData user_group[USERS_PER_INDEX];

    for (size_t start_index = 0; start_index < user_count && !user_found; start_index += USERS_PER_INDEX) {
        char key[100];
        snprintf(key, sizeof(key), "users_data_%" PRIuPTR, start_index / USERS_PER_INDEX);
        size_t expected_size = sizeof(user_group);
        err = nvs_get_blob(my_handle, key, user_group, &expected_size);

        for (size_t i = 0; i < USERS_PER_INDEX && start_index + i < user_count; i++) {
            if (strcmp(user_group[i].uuid, uuid) == 0) {
                user_found = true;

                // Shift subsequent users to overwrite the deleted user
                for (size_t j = i; j < USERS_PER_INDEX - 1; j++) {
                    user_group[j] = user_group[j+1];
                }
                memset(&user_group[USERS_PER_INDEX - 1], 0, sizeof(UserData)); // Clear the last element
                err = nvs_set_blob(my_handle, key, user_group, sizeof(user_group));
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to store updated users data with error: %d", err);
                }
                break;
            }
        }
    }

    // Update the user count if a user was deleted
    if (user_found) {
        store_u32("auth_user_count", user_count - 1);
        ESP_LOGI(TAG, "Deleted user with UUID: %s. Total users: %lu", uuid, get_u32("auth_user_count", 0));
    } else {
        ESP_LOGW(TAG, "User with UUID: %s not found", uuid);
    }

    nvs_commit(my_handle);
    nvs_close(my_handle);
}

void modify_user_from_flash(const char *uuid, const char *newName, const char *newPin) {
    ESP_LOGI(TAG, "Modifying user in flash: UUID=%s", uuid);
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS with error: %d", err);
        return;
    }

    size_t user_count = get_u32("auth_user_count", 0);
    bool user_modified = false;

    UserData user_group[USERS_PER_INDEX];

    for (size_t start_index = 0; start_index < user_count && !user_modified; start_index += USERS_PER_INDEX) {
        char key[100];
        snprintf(key, sizeof(key), "users_data_%" PRIuPTR, start_index / USERS_PER_INDEX);
        size_t expected_size = sizeof(user_group);
        err = nvs_get_blob(my_handle, key, user_group, &expected_size);

        for (size_t i = 0; i < USERS_PER_INDEX && start_index + i < user_count; i++) {
            if (strcmp(user_group[i].uuid, uuid) == 0) {
                // Modify the user
                strncpy(user_group[i].name, newName, sizeof(user_group[i].name));
                strncpy(user_group[i].pin, newPin, sizeof(user_group[i].pin));

                // Store the modified user_group back to NVS
                err = nvs_set_blob(my_handle, key, user_group, sizeof(user_group));
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to store modified users data with error: %d", err);
                } else {
                    ESP_LOGI(TAG, "Modified user with UUID: %s", uuid);
                }
                user_modified = true;
                break;
            }
        }
    }

    if (!user_modified) {
        ESP_LOGW(TAG, "User with UUID: %s not found", uuid);
    }

    nvs_commit(my_handle);
    nvs_close(my_handle);
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

char* find_pin_in_flash(const char* pin) 
{
    uint32_t user_count = get_u32("auth_user_count", 0);
    ESP_LOGI(TAG, "Total User Count: %" PRIu32, user_count);

    for (uint32_t i = 0; i < user_count; i++) { // Start from 0 as it's 0-based
        cJSON *user = load_user_from_flash(i);
        if (!user) continue;

        cJSON *pin_json = cJSON_GetObjectItem(user, "pin");
        if (pin_json && (strcmp(pin_json->valuestring, pin) == 0)) {
            char *uuid = strdup(cJSON_GetObjectItem(user, "uuid")->valuestring);
            cJSON_Delete(user);
            return uuid;
        }
        cJSON_Delete(user);
    }
    return NULL;
}