/**
 * RF Registry - manages learned 433MHz remote fob codes.
 *
 * Stored in NVS as JSON string under key "rf_fobs".
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "store.h"
#include "rf_registry.h"

#define RF_REGISTRY_TAG      "rf_registry"
#define RF_NVS_KEY           "rf_fobs"
#define RF_USER_MAX_COUNT    256
#define RF_NAME_MAX          32

typedef struct {
    char id[37];
    char code[9]; /* 6 hex chars + null, but keep a bit more */
    char name[RF_NAME_MAX];
    uint32_t sequence;
    uint64_t created_ms;
    uint64_t updated_ms;
    size_t last_pulse_count;
} rf_user_t;

static rf_user_t *rf_users = NULL;
static size_t rf_user_count = 0;
static size_t rf_user_capacity = 0;
static bool rf_initialised = false;
static bool rf_registration_active = false;
static uint32_t rf_pending = 0;
static char rf_last_duplicate[9] = "";
static SemaphoreHandle_t rf_mutex = NULL;

static uint64_t now_ms(void) { return esp_timer_get_time() / 1000ULL; }

static void rf_ensure_mutex(void) {
    if (!rf_mutex) {
        rf_mutex = xSemaphoreCreateMutex();
    }
}

static void clear_users_locked(void) {
    if (rf_users && rf_user_capacity) {
        memset(rf_users, 0, rf_user_capacity * sizeof(*rf_users));
    }
    rf_user_count = 0;
}

static bool ensure_capacity_locked(size_t required) {
    if (required <= rf_user_capacity) return true;

    size_t new_cap = rf_user_capacity ? rf_user_capacity : 16;
    while (new_cap < required && new_cap < RF_USER_MAX_COUNT) {
        new_cap *= 2;
    }
    if (new_cap > RF_USER_MAX_COUNT) new_cap = RF_USER_MAX_COUNT;
    if (new_cap < required) return false;

    rf_user_t *new_users = realloc(rf_users, new_cap * sizeof(*rf_users));
    if (!new_users) return false;
    if (new_cap > rf_user_capacity) {
        memset(new_users + rf_user_capacity, 0, (new_cap - rf_user_capacity) * sizeof(*new_users));
    }
    rf_users = new_users;
    rf_user_capacity = new_cap;
    return true;
}

static void to_hex_code(uint32_t code, char *out, size_t len) {
    if (!out || len < 9) return;
    snprintf(out, len, "%06lX", (unsigned long)(code & 0xFFFFFF));
}

static void rf_generate_id(char *out, size_t len) {
    if (!out || len < 9) return;
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    snprintf(out, len, "%08lX%08lX", (unsigned long)r1, (unsigned long)r2);
}

static void persist_locked(void) {
    cJSON *arr = cJSON_CreateArray();
    if (!arr) return;
    for (size_t i = 0; i < rf_user_count; i++) {
        rf_user_t *u = &rf_users[i];
        cJSON *obj = cJSON_CreateObject();
        if (!obj) continue;
        cJSON_AddStringToObject(obj, "id", u->id);
        cJSON_AddStringToObject(obj, "code", u->code);
        cJSON_AddStringToObject(obj, "name", u->name);
        cJSON_AddNumberToObject(obj, "sequence", (double)u->sequence);
        cJSON_AddNumberToObject(obj, "created_ms", (double)u->created_ms);
        cJSON_AddNumberToObject(obj, "updated_ms", (double)u->updated_ms);
        cJSON_AddNumberToObject(obj, "pulse_count", (double)u->last_pulse_count);
        cJSON_AddItemToArray(arr, obj);
    }
    char *json = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    if (json) {
        store_char(RF_NVS_KEY, json);
        free(json);
    }
}

static void load_from_nvs_locked(void) {
    char *json = get_char(RF_NVS_KEY);
    if (!json || json[0] == '\0') {
        free(json);
        return;
    }
    cJSON *arr = cJSON_Parse(json);
    free(json);
    if (!cJSON_IsArray(arr)) {
        cJSON_Delete(arr);
        return;
    }
    clear_users_locked();
    size_t count = cJSON_GetArraySize(arr);
    if (!ensure_capacity_locked(count)) {
        cJSON_Delete(arr);
        return;
    }
    for (size_t i = 0; i < count; i++) {
        const cJSON *obj = cJSON_GetArrayItem(arr, i);
        if (!cJSON_IsObject(obj)) continue;
        rf_user_t tmp = {0};
        const cJSON *id = cJSON_GetObjectItemCaseSensitive(obj, "id");
        const cJSON *code = cJSON_GetObjectItemCaseSensitive(obj, "code");
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(obj, "name");
        const cJSON *seq = cJSON_GetObjectItemCaseSensitive(obj, "sequence");
        const cJSON *created = cJSON_GetObjectItemCaseSensitive(obj, "created_ms");
        const cJSON *updated = cJSON_GetObjectItemCaseSensitive(obj, "updated_ms");
        const cJSON *pulse = cJSON_GetObjectItemCaseSensitive(obj, "pulse_count");

        if (!cJSON_IsString(id) || !cJSON_IsString(code)) continue;
        strlcpy(tmp.id, id->valuestring, sizeof(tmp.id));
        strlcpy(tmp.code, code->valuestring, sizeof(tmp.code));
        if (cJSON_IsString(name) && name->valuestring) {
            strlcpy(tmp.name, name->valuestring, sizeof(tmp.name));
        }
        tmp.sequence = cJSON_IsNumber(seq) ? (uint32_t)seq->valuedouble : (uint32_t)(i + 1);
        tmp.created_ms = cJSON_IsNumber(created) ? (uint64_t)created->valuedouble : now_ms();
        tmp.updated_ms = cJSON_IsNumber(updated) ? (uint64_t)updated->valuedouble : tmp.created_ms;
        tmp.last_pulse_count = cJSON_IsNumber(pulse) ? (size_t)pulse->valuedouble : 0;

        if (tmp.name[0] == '\0') {
            snprintf(tmp.name, sizeof(tmp.name), "Remote Fob %lu", (unsigned long)(tmp.sequence));
        }
        rf_users[rf_user_count++] = tmp;
    }
    cJSON_Delete(arr);
}

void rf_registry_init(void) {
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    if (!rf_initialised) {
        rf_initialised = true;
        load_from_nvs_locked();
    }
    xSemaphoreGive(rf_mutex);
}

bool rf_registry_is_active(void) {
    return rf_registration_active;
}

esp_err_t rf_registration_start(void) {
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    if (rf_registration_active) {
        xSemaphoreGive(rf_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    rf_registration_active = true;
    rf_pending = 0;
    rf_last_duplicate[0] = '\0';
    xSemaphoreGive(rf_mutex);
    return ESP_OK;
}

esp_err_t rf_registration_stop(void) {
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    if (!rf_registration_active) {
        xSemaphoreGive(rf_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    rf_registration_active = false;
    rf_pending = 0;
    xSemaphoreGive(rf_mutex);
    return ESP_OK;
}

static bool code_exists_locked(const char *code) {
    for (size_t i = 0; i < rf_user_count; i++) {
        if (strcmp(rf_users[i].code, code) == 0) {
            return true;
        }
    }
    return false;
}

static rf_user_t *find_by_id_locked(const char *id) {
    if (!id) return NULL;
    for (size_t i = 0; i < rf_user_count; i++) {
        if (strcmp(rf_users[i].id, id) == 0) {
            return &rf_users[i];
        }
    }
    return NULL;
}

void rf_registry_on_code(uint32_t code, size_t pulse_count) {
    if (!rf_registration_active) return;
    char code_hex[9] = {0};
    to_hex_code(code, code_hex, sizeof(code_hex));

    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);

    if (code_exists_locked(code_hex)) {
        strlcpy(rf_last_duplicate, code_hex, sizeof(rf_last_duplicate));
        xSemaphoreGive(rf_mutex);
        return;
    }

    if (!ensure_capacity_locked(rf_user_count + 1)) {
        xSemaphoreGive(rf_mutex);
        ESP_LOGW(RF_REGISTRY_TAG, "Registry full, cannot add code %s", code_hex);
        return;
    }

    rf_user_t *u = &rf_users[rf_user_count++];
    memset(u, 0, sizeof(*u));
    rf_generate_id(u->id, sizeof(u->id));
    strlcpy(u->code, code_hex, sizeof(u->code));
    snprintf(u->name, sizeof(u->name), "Remote Fob %lu", (unsigned long)(rf_user_count));
    u->sequence = rf_user_count;
    u->created_ms = now_ms();
    u->updated_ms = u->created_ms;
    u->last_pulse_count = pulse_count;
    rf_pending++;

    persist_locked();
    xSemaphoreGive(rf_mutex);
}

esp_err_t rf_registry_update_name(const char *id, const char *name) {
    if (!id || !name || name[0] == '\0') return ESP_ERR_INVALID_ARG;
    if (strlen(name) >= RF_NAME_MAX) return ESP_ERR_INVALID_SIZE;

    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    rf_user_t *u = find_by_id_locked(id);
    if (!u) {
        xSemaphoreGive(rf_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    strlcpy(u->name, name, sizeof(u->name));
    u->updated_ms = now_ms();
    persist_locked();
    xSemaphoreGive(rf_mutex);
    return ESP_OK;
}

esp_err_t rf_registry_remove(const char *id) {
    if (!id) return ESP_ERR_INVALID_ARG;
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    ssize_t idx = -1;
    for (size_t i = 0; i < rf_user_count; i++) {
        if (strcmp(rf_users[i].id, id) == 0) {
            idx = (ssize_t)i;
            break;
        }
    }
    if (idx < 0) {
        xSemaphoreGive(rf_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    for (size_t i = idx; i + 1 < rf_user_count; i++) {
        rf_users[i] = rf_users[i + 1];
    }
    rf_user_count--;
    persist_locked();
    xSemaphoreGive(rf_mutex);
    return ESP_OK;
}

static cJSON *serialize_state_locked(void) {
    cJSON *obj = cJSON_CreateObject();
    if (!obj) return NULL;
    cJSON_AddBoolToObject(obj, "registrationActive", rf_registration_active);
    cJSON_AddNumberToObject(obj, "registrationPending", (double)rf_pending);
    cJSON_AddStringToObject(obj, "lastDuplicateCode", rf_last_duplicate);

    cJSON *arr = cJSON_CreateArray();
    if (arr) {
        for (size_t i = 0; i < rf_user_count; i++) {
            rf_user_t *u = &rf_users[i];
            cJSON *uobj = cJSON_CreateObject();
            if (!uobj) continue;
            cJSON_AddStringToObject(uobj, "id", u->id);
            cJSON_AddStringToObject(uobj, "code", u->code);
            cJSON_AddStringToObject(uobj, "name", u->name);
            cJSON_AddNumberToObject(uobj, "sequence", (double)u->sequence);
            cJSON_AddNumberToObject(uobj, "created_at_ms", (double)u->created_ms);
            cJSON_AddNumberToObject(uobj, "updated_at_ms", (double)u->updated_ms);
            cJSON_AddNumberToObject(uobj, "pulse_count", (double)u->last_pulse_count);
            cJSON_AddItemToArray(arr, uobj);
        }
        cJSON_AddItemToObject(obj, "users", arr);
    }
    return obj;
}

cJSON *rf_state_snapshot(void) {
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    cJSON *snap = serialize_state_locked();
    xSemaphoreGive(rf_mutex);
    return snap;
}
