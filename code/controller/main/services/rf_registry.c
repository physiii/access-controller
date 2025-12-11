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
#include "freertos/timers.h"
#include "store.h"
#include "rf_registry.h"
#include "automation.h"

/* lock actions */
void arm_lock(int channel, bool arm, bool alert);
void beep_keypad(int beeps, int channel);

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
    char mode[12];          /* toggle, momentary, exit, power_on, power_off */
    int channel_mask;       /* bit0 -> ch1, bit1 -> ch2 */
    int exit_seconds;       /* only used for exit mode */
    bool alert;             /* play buzzer */
} rf_user_t;

static rf_user_t *rf_users = NULL;
static size_t rf_user_count = 0;
static size_t rf_user_capacity = 0;
static bool rf_initialised = false;
static bool rf_registration_active = false;
static uint32_t rf_pending = 0;
static char rf_last_duplicate[9] = "";
static SemaphoreHandle_t rf_mutex = NULL;
static TimerHandle_t rf_exit_timers[2] = {0};
static TimerHandle_t rf_momentary_timers[2] = {0};
static bool rf_toggle_state[2] = {false, false};

/* forward */
static void log_action(const rf_user_t *u, const char *action);

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

static bool rf_ensure_capacity_locked(size_t required) {
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

static void rf_persist_locked(void) {
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
        cJSON_AddStringToObject(obj, "mode", u->mode);
        cJSON_AddNumberToObject(obj, "channel_mask", (double)u->channel_mask);
        cJSON_AddNumberToObject(obj, "exit_seconds", (double)u->exit_seconds);
        cJSON_AddBoolToObject(obj, "alert", u->alert);
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
    if (!rf_ensure_capacity_locked(count)) {
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
        const cJSON *mode = cJSON_GetObjectItemCaseSensitive(obj, "mode");
        const cJSON *cmask = cJSON_GetObjectItemCaseSensitive(obj, "channel_mask");
        const cJSON *exit_s = cJSON_GetObjectItemCaseSensitive(obj, "exit_seconds");
        const cJSON *alert = cJSON_GetObjectItemCaseSensitive(obj, "alert");

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
        strlcpy(tmp.mode, (cJSON_IsString(mode) && mode->valuestring) ? mode->valuestring : "toggle", sizeof(tmp.mode));
        tmp.channel_mask = cJSON_IsNumber(cmask) ? (int)cmask->valuedouble : 0x1; /* default ch1 */
        tmp.exit_seconds = cJSON_IsNumber(exit_s) ? (int)exit_s->valuedouble : 4;
        tmp.alert = cJSON_IsBool(alert) ? cJSON_IsTrue(alert) : true;

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

    if (!rf_ensure_capacity_locked(rf_user_count + 1)) {
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
    strlcpy(u->mode, "toggle", sizeof(u->mode));
    u->channel_mask = 0x1;   /* default channel 1 */
    u->exit_seconds = 4;
    u->alert = true;
    rf_pending++;

    rf_persist_locked();
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
    rf_persist_locked();
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
    rf_persist_locked();
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
            cJSON_AddStringToObject(uobj, "mode", u->mode);
            cJSON_AddNumberToObject(uobj, "channel_mask", (double)u->channel_mask);
            cJSON_AddNumberToObject(uobj, "exit_seconds", (double)u->exit_seconds);
            cJSON_AddBoolToObject(uobj, "alert", u->alert);
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

static bool is_valid_mode(const char *mode) {
    return mode &&
        (!strcmp(mode, "toggle") ||
         !strcmp(mode, "momentary") ||
         !strcmp(mode, "exit") ||
         !strcmp(mode, "power_on") ||
         !strcmp(mode, "power_off"));
}

static void timer_rearm_callback(TimerHandle_t xTimer) {
    intptr_t ch = (intptr_t)pvTimerGetTimerID(xTimer);
    int channel = (int)ch;
    if (channel < 1 || channel > 2) return;
    arm_lock(channel, true, false);
    ESP_LOGI(RF_REGISTRY_TAG, "[RF ACTION] ch=%d re-arm after timer", channel);
}

static void start_rearm_timer(TimerHandle_t *timer_slot, int channel, int seconds) {
    if (channel < 1 || channel > 2) return;
    int idx = channel - 1;
    if (!timer_slot[idx]) {
        timer_slot[idx] = xTimerCreate("rf_rearm", pdMS_TO_TICKS(seconds * 1000), pdFALSE, (void *)(intptr_t)channel, timer_rearm_callback);
    }
    if (timer_slot[idx]) {
        xTimerStop(timer_slot[idx], 0);
        xTimerChangePeriod(timer_slot[idx], pdMS_TO_TICKS(seconds * 1000), 0);
        xTimerStart(timer_slot[idx], 0);
    }
}

static void apply_mode_action(const rf_user_t *u) {
    if (!u) return;
    for (int bit = 0; bit < 2; bit++) {
        if ((u->channel_mask & (1 << bit)) == 0) continue;
        int channel = bit + 1;
        if (!strcmp(u->mode, "toggle")) {
            rf_toggle_state[bit] = !rf_toggle_state[bit];
            arm_lock(channel, rf_toggle_state[bit], false);
            log_action(u, rf_toggle_state[bit] ? "toggle->on" : "toggle->off");
            if (u->alert) beep_keypad(1, channel);
        } else if (!strcmp(u->mode, "momentary")) {
            arm_lock(channel, false, false); // off while signal present
            start_rearm_timer(rf_momentary_timers, channel, u->exit_seconds > 0 ? u->exit_seconds : 1);
            log_action(u, "momentary (off then re-arm)");
            if (u->alert) beep_keypad(1, channel);
        } else if (!strcmp(u->mode, "exit")) {
            arm_lock(channel, false, false);
            start_rearm_timer(rf_exit_timers, channel, u->exit_seconds > 0 ? u->exit_seconds : 4);
            log_action(u, "exit pulse");
            if (u->alert) beep_keypad(1, channel);
        } else if (!strcmp(u->mode, "power_on")) {
            arm_lock(channel, true, false);
            log_action(u, "power_on");
            if (u->alert) beep_keypad(1, channel);
        } else if (!strcmp(u->mode, "power_off")) {
            arm_lock(channel, false, false);
            log_action(u, "power_off");
            if (u->alert) beep_keypad(1, channel);
        } else {
            log_action(u, "unknown");
        }
    }
}

esp_err_t rf_registry_update_config(const char *id, const char *mode, int channel_mask, int exit_seconds, bool alert) {
    if (!id || !mode) return ESP_ERR_INVALID_ARG;
    if (!is_valid_mode(mode)) return ESP_ERR_INVALID_ARG;
    if (channel_mask <= 0 || channel_mask > 0x3) return ESP_ERR_INVALID_ARG;
    if (exit_seconds <= 0) exit_seconds = 4;
    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    rf_user_t *u = find_by_id_locked(id);
    if (!u) {
        xSemaphoreGive(rf_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    strlcpy(u->mode, mode, sizeof(u->mode));
    u->channel_mask = channel_mask;
    u->exit_seconds = exit_seconds;
    u->alert = alert;
    u->updated_ms = now_ms();
    rf_persist_locked();
    xSemaphoreGive(rf_mutex);
    return ESP_OK;
}

static void log_action(const rf_user_t *u, const char *action) {
    ESP_LOGI(RF_REGISTRY_TAG, "[RF ACTION] code=%s mode=%s chmask=0x%x action=%s exit_s=%d alert=%d name=%s",
             u->code, u->mode, u->channel_mask, action, u->exit_seconds, u->alert ? 1 : 0, u->name);
    char msg[128];
    snprintf(msg, sizeof(msg), "RF %s (%s) chmask=0x%x %s", u->name, u->code, u->channel_mask, action);
    automation_record_log(msg);
}

static void apply_action_for_user(const rf_user_t *u) {
    apply_mode_action(u);
}

bool rf_registry_handle_code(uint32_t code) {
    char code_hex[9] = {0};
    to_hex_code(code, code_hex, sizeof(code_hex));

    rf_ensure_mutex();
    xSemaphoreTake(rf_mutex, portMAX_DELAY);
    rf_user_t *u = NULL;
    for (size_t i = 0; i < rf_user_count; i++) {
        if (strcmp(rf_users[i].code, code_hex) == 0) {
            u = &rf_users[i];
            break;
        }
    }
    if (!u) {
        xSemaphoreGive(rf_mutex);
        ESP_LOGW(RF_REGISTRY_TAG, "Unknown RF code %s (no action)", code_hex);
        return false;
    }
    u->last_pulse_count = 0;
    u->updated_ms = now_ms();
    rf_persist_locked();
    xSemaphoreGive(rf_mutex);

    apply_action_for_user(u);
    return true;
}
