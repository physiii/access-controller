#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "automation.h"
#include "wiegand_registry.h"
#include "store.h"

static const char *LOG_TAG_WIEGAND_REGISTRY = "wiegand_registry";
static const char *NVS_KEY = "wiegand_users";

static wiegand_user_t *s_users = NULL;
static size_t s_user_count = 0;
static size_t s_user_capacity = 0;
static bool s_initialised = false;
static SemaphoreHandle_t s_mutex = NULL;

static uint64_t current_time_ms(void) {
    return esp_timer_get_time() / 1000ULL;
}

static void ensure_mutex(void) {
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
    }
}

static void clear_users(void) {
    if (s_users && s_user_capacity > 0) {
        memset(s_users, 0, s_user_capacity * sizeof(*s_users));
    }
    s_user_count = 0;
}

static bool ensure_capacity_locked(size_t required) {
    if (required <= s_user_capacity) {
        return true;
    }

    size_t new_capacity = s_user_capacity ? s_user_capacity : 16;
    while (new_capacity < required && new_capacity < WIEGAND_USER_MAX_COUNT) {
        new_capacity *= 2;
    }

    if (new_capacity > WIEGAND_USER_MAX_COUNT) {
        new_capacity = WIEGAND_USER_MAX_COUNT;
    }

    if (new_capacity < required) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "Registry capacity limit reached (%u)", WIEGAND_USER_MAX_COUNT);
        return false;
    }

    wiegand_user_t *new_users = realloc(s_users, new_capacity * sizeof(*s_users));
    if (!new_users) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Failed to expand registry to %zu entries", required);
        return false;
    }

    size_t old_capacity = s_user_capacity;
    s_users = new_users;
    s_user_capacity = new_capacity;
    if (s_user_capacity > old_capacity) {
        memset(s_users + old_capacity, 0, (s_user_capacity - old_capacity) * sizeof(*s_users));
    }
    return true;
}

static void assign_defaults(wiegand_user_t *user) {
    if (!user) return;
    if (user->name[0] == '\0') {
        snprintf(user->name, sizeof(user->name), "User %lu", (unsigned long)(user->sequence + 1));
    }
    if (user->status != WIEGAND_USER_STATUS_ACTIVE &&
        user->status != WIEGAND_USER_STATUS_DISABLED) {
        user->status = WIEGAND_USER_STATUS_ACTIVE;
    }
}

static cJSON *serialize_user(const wiegand_user_t *user) {
    if (!user) return NULL;
    cJSON *obj = cJSON_CreateObject();
    if (!obj) return NULL;

    cJSON_AddStringToObject(obj, "id", user->id);
    cJSON_AddStringToObject(obj, "code", user->code);
    cJSON_AddStringToObject(obj, "name", user->name);
    cJSON_AddNumberToObject(obj, "channel", user->channel);
    cJSON_AddNumberToObject(obj, "status", user->status);
    cJSON_AddNumberToObject(obj, "sequence", user->sequence);
    cJSON_AddNumberToObject(obj, "created_at_ms", (double)user->created_at_ms);
    cJSON_AddNumberToObject(obj, "updated_at_ms", (double)user->updated_at_ms);
    return obj;
}

static bool parse_string_field(const cJSON *obj, const char *key, char *out, size_t len) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (!cJSON_IsString(item) || !item->valuestring) {
        return false;
    }
    strlcpy(out, item->valuestring, len);
    return true;
}

static bool deserialize_user(const cJSON *obj, wiegand_user_t *out_user) {
    if (!cJSON_IsObject(obj) || !out_user) {
        return false;
    }

    memset(out_user, 0, sizeof(*out_user));

    if (!parse_string_field(obj, "id", out_user->id, sizeof(out_user->id))) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "User missing id");
        return false;
    }
    if (!parse_string_field(obj, "code", out_user->code, sizeof(out_user->code))) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "User missing code");
        return false;
    }
    parse_string_field(obj, "name", out_user->name, sizeof(out_user->name));

    const cJSON *channel = cJSON_GetObjectItemCaseSensitive(obj, "channel");
    const cJSON *status = cJSON_GetObjectItemCaseSensitive(obj, "status");
    const cJSON *sequence = cJSON_GetObjectItemCaseSensitive(obj, "sequence");
    const cJSON *created_at = cJSON_GetObjectItemCaseSensitive(obj, "created_at_ms");
    const cJSON *updated_at = cJSON_GetObjectItemCaseSensitive(obj, "updated_at_ms");

    out_user->channel = cJSON_IsNumber(channel) ? (uint8_t)channel->valuedouble : 0;
    out_user->status = cJSON_IsNumber(status) ? (wiegand_user_status_t)status->valuedouble : WIEGAND_USER_STATUS_ACTIVE;
    out_user->sequence = cJSON_IsNumber(sequence) ? (uint32_t)sequence->valuedouble : 0;
    out_user->created_at_ms = cJSON_IsNumber(created_at) ? (uint64_t)created_at->valuedouble : current_time_ms();
    out_user->updated_at_ms = cJSON_IsNumber(updated_at) ? (uint64_t)updated_at->valuedouble : out_user->created_at_ms;

    assign_defaults(out_user);
    return true;
}

static void sort_users(void) {
    if (s_user_count < 2) return;
    for (size_t i = 0; i < s_user_count - 1; i++) {
        for (size_t j = i + 1; j < s_user_count; j++) {
            if (s_users[i].sequence > s_users[j].sequence) {
                wiegand_user_t tmp = s_users[i];
                s_users[i] = s_users[j];
                s_users[j] = tmp;
            }
        }
    }
}

static uint32_t next_sequence(void) {
    uint32_t max_seq = 0;
    for (size_t i = 0; i < s_user_count; i++) {
        if (s_users[i].sequence > max_seq) {
            max_seq = s_users[i].sequence;
        }
    }
    return max_seq + 1;
}

static void persist_locked(void);

void wiegand_registry_init(void) {
    ensure_mutex();
    if (s_initialised) {
        return;
    }
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    free(s_users);
    s_users = NULL;
    s_user_capacity = 0;
    clear_users();
    s_initialised = true;
    xSemaphoreGive(s_mutex);
    if (wiegand_registry_reload() != ESP_OK) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "Initial load failed; continuing with empty registry");
    }
}

bool wiegand_registry_is_initialised(void) {
    return s_initialised;
}

size_t wiegand_registry_count(void) {
    return s_user_count;
}

const wiegand_user_t *wiegand_registry_get(size_t index) {
    if (index >= s_user_count || !s_users) {
        return NULL;
    }
    return &s_users[index];
}

static ssize_t find_index_by_code(const char *code) {
    if (!code) return -1;
    for (size_t i = 0; i < s_user_count; i++) {
        if (strcmp(s_users[i].code, code) == 0) {
            return (ssize_t)i;
        }
    }
    return -1;
}

static ssize_t find_index_by_id(const char *id) {
    if (!id) return -1;
    for (size_t i = 0; i < s_user_count; i++) {
        if (strcmp(s_users[i].id, id) == 0) {
            return (ssize_t)i;
        }
    }
    return -1;
}

const wiegand_user_t *wiegand_registry_find_by_code(const char *code) {
    ssize_t idx = find_index_by_code(code);
    if (idx < 0) return NULL;
    return &s_users[idx];
}

const wiegand_user_t *wiegand_registry_find_by_id(const char *id) {
    ssize_t idx = find_index_by_id(id);
    if (idx < 0) return NULL;
    return &s_users[idx];
}

static void persist_locked(void) {
    cJSON *array = cJSON_CreateArray();
    if (!array) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Failed to allocate array for persistence");
        return;
    }

    for (size_t i = 0; i < s_user_count; i++) {
        cJSON *entry = serialize_user(&s_users[i]);
        if (entry) {
            cJSON_AddItemToArray(array, entry);
        }
    }

    char *json = cJSON_PrintUnformatted(array);
    cJSON_Delete(array);
    if (!json) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Failed to serialise registry");
        return;
    }

    if (store_char(NVS_KEY, json) != ESP_OK) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Failed to persist Wiegand registry");
    }
    free(json);
}

static esp_err_t load_locked(void) {
    char *json_str = get_char(NVS_KEY);
    if (!json_str) {
        return ESP_ERR_NO_MEM;
    }

    if (json_str[0] == '\0') {
        free(json_str);
        clear_users();
        return ESP_OK;
    }

    cJSON *array = cJSON_Parse(json_str);
    free(json_str);
    if (!array || !cJSON_IsArray(array)) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "Stored wiegand_users is not an array");
        cJSON_Delete(array);
        return ESP_FAIL;
    }

    size_t count = cJSON_GetArraySize(array);
    if (count > WIEGAND_USER_MAX_COUNT) {
        ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "Stored users exceed maximum (%zu > %d); truncating", count, WIEGAND_USER_MAX_COUNT);
        count = WIEGAND_USER_MAX_COUNT;
    }

    clear_users();
    if (!ensure_capacity_locked(count)) {
        cJSON_Delete(array);
        return ESP_ERR_NO_MEM;
    }

    for (size_t i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        if (item) {
            wiegand_user_t user;
            if (deserialize_user(item, &user)) {
                if (s_user_count < WIEGAND_USER_MAX_COUNT) {
                    if (!ensure_capacity_locked(s_user_count + 1)) {
                        cJSON_Delete(array);
                        return ESP_ERR_NO_MEM;
                    }
                    s_users[s_user_count++] = user;
                } else {
                    ESP_LOGW(LOG_TAG_WIEGAND_REGISTRY, "Registry full while loading; truncating");
                    break;
                }
            }
        }
    }

    cJSON_Delete(array);
    sort_users();
    return ESP_OK;
}

esp_err_t wiegand_registry_reload(void) {
    ensure_mutex();
    if (!s_mutex) {
        return ESP_ERR_NO_MEM;
    }
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t result = load_locked();
    xSemaphoreGive(s_mutex);
    return result;
}

esp_err_t wiegand_registry_save(void) {
    ensure_mutex();
    if (!s_mutex) {
        return ESP_ERR_NO_MEM;
    }
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    persist_locked();
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

static void generate_id(char *buffer, size_t len) {
    const char *alphabet = "0123456789abcdef";
    for (size_t i = 0; i + 1 < len; i++) {
        uint32_t rnd = esp_random();
        buffer[i] = alphabet[rnd % 16];
    }
    buffer[len - 1] = '\0';
}

esp_err_t wiegand_registry_add(const char *code, uint8_t channel, wiegand_user_t *out_user) {
    if (!code || code[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    ensure_mutex();
    if (!s_mutex) {
        return ESP_ERR_NO_MEM;
    }

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    if (!s_initialised) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Registry not initialised");
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    if (s_user_count >= WIEGAND_USER_MAX_COUNT) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    if (find_index_by_code(code) >= 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    if (!ensure_capacity_locked(s_user_count + 1)) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    wiegand_user_t user = {
        .channel = channel,
        .status = WIEGAND_USER_STATUS_ACTIVE,
        .sequence = next_sequence(),
        .created_at_ms = current_time_ms(),
        .updated_at_ms = current_time_ms(),
    };
    strlcpy(user.code, code, sizeof(user.code));
    generate_id(user.id, sizeof(user.id));
    assign_defaults(&user);

    s_users[s_user_count++] = user;
    sort_users();
    persist_locked();
    xSemaphoreGive(s_mutex);

    if (out_user) {
        *out_user = user;
    }
    return ESP_OK;
}

static esp_err_t update_user_locked(size_t idx, const wiegand_user_t *replacement) {
    if (idx >= s_user_count || !replacement) {
        return ESP_ERR_INVALID_ARG;
    }
    s_users[idx] = *replacement;
    s_users[idx].updated_at_ms = current_time_ms();
    persist_locked();
    return ESP_OK;
}

esp_err_t wiegand_registry_update_name(const char *id, const char *name) {
    if (!id || !name) {
        return ESP_ERR_INVALID_ARG;
    }
    ensure_mutex();
    if (!s_mutex) return ESP_ERR_NO_MEM;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    ssize_t idx = find_index_by_id(id);
    if (idx < 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    strlcpy(s_users[idx].name, name, sizeof(s_users[idx].name));
    assign_defaults(&s_users[idx]);
    esp_err_t result = update_user_locked((size_t)idx, &s_users[idx]);
    xSemaphoreGive(s_mutex);
    return result;
}

esp_err_t wiegand_registry_update_status(const char *id, wiegand_user_status_t status) {
    ensure_mutex();
    if (!s_mutex) return ESP_ERR_NO_MEM;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    ssize_t idx = find_index_by_id(id);
    if (idx < 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    s_users[idx].status = status;
    assign_defaults(&s_users[idx]);
    esp_err_t result = update_user_locked((size_t)idx, &s_users[idx]);
    xSemaphoreGive(s_mutex);
    return result;
}

esp_err_t wiegand_registry_remove(const char *id) {
    if (!id) return ESP_ERR_INVALID_ARG;
    ensure_mutex();
    if (!s_mutex) return ESP_ERR_NO_MEM;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    ssize_t idx = find_index_by_id(id);
    if (idx < 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    for (size_t i = (size_t)idx; i + 1 < s_user_count; i++) {
        s_users[i] = s_users[i + 1];
    }
    if (s_user_count > 0) {
        s_user_count--;
    }
    persist_locked();
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

cJSON *wiegand_registry_snapshot(void) {
    if (!s_initialised) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Registry not initialised");
        return NULL;
    }

    cJSON *array = cJSON_CreateArray();
    if (!array) {
        ESP_LOGE(LOG_TAG_WIEGAND_REGISTRY, "Failed to create snapshot array");
        return NULL;
    }

    if (!s_mutex || xSemaphoreTake(s_mutex, portMAX_DELAY) != pdTRUE) {
        cJSON_Delete(array);
        return NULL;
    }

    for (size_t i = 0; i < s_user_count; i++) {
        cJSON *obj = serialize_user(&s_users[i]);
        if (obj) {
            cJSON_AddItemToArray(array, obj);
        }
    }

    xSemaphoreGive(s_mutex);
    return array;
}

