#include "log_store.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"

#define LOG_STORE_NAMESPACE "syslog"
#define LOG_STORE_KEY       "ring"
#define LOG_STORE_BLOB_VERSION 0x00010000u

typedef struct {
    uint64_t timestamp_ms;
    int64_t unix_time;
    char message[LOG_STORE_MESSAGE_MAX];
} persisted_log_entry_t;

typedef struct {
    uint32_t version;
    uint32_t count;
} persisted_log_header_t;

typedef struct {
    uint32_t count;
    uint32_t next_index;
    persisted_log_entry_t entries[LOG_STORE_CAPACITY];
} log_ring_t;

typedef struct {
    uint32_t count;
    uint32_t next_index;
    struct {
        uint64_t timestamp_ms;
        int64_t unix_time;
        char message[160];
    } entries[LOG_STORE_CAPACITY];
} legacy_log_ring_t;

#define LOG_STORE_BLOB_SIZE (sizeof(persisted_log_header_t) + LOG_STORE_CAPACITY * sizeof(persisted_log_entry_t))
#define LOG_STORE_BUFFER_SIZE ((LOG_STORE_BLOB_SIZE > sizeof(legacy_log_ring_t)) ? LOG_STORE_BLOB_SIZE : sizeof(legacy_log_ring_t))

static const char *TAG = "log_store";

static log_ring_t s_ring;
static bool s_loaded = false;
static SemaphoreHandle_t s_mutex = NULL;
static uint8_t s_blob_buffer[LOG_STORE_BUFFER_SIZE];
static uint32_t s_dirty_entries = 0;
static uint64_t s_last_flush_timestamp = 0;

#define LOG_STORE_FLUSH_THRESHOLD 5
#define LOG_STORE_FLUSH_INTERVAL_MS 60000ULL

static esp_err_t open_handle(nvs_handle_t *handle) {
    return nvs_open(LOG_STORE_NAMESPACE, NVS_READWRITE, handle);
}

static void ring_reset(void) {
    memset(&s_ring, 0, sizeof(s_ring));
}

static void ring_append_internal(const persisted_log_entry_t *entry) {
    s_ring.entries[s_ring.next_index] = *entry;
    s_ring.next_index = (s_ring.next_index + 1) % LOG_STORE_CAPACITY;
    if (s_ring.count < LOG_STORE_CAPACITY) {
        s_ring.count++;
    }
}

static void migrate_legacy_blob(const legacy_log_ring_t *legacy) {
    ring_reset();

    uint32_t count = legacy->count;
    if (count > LOG_STORE_CAPACITY) {
        count = LOG_STORE_CAPACITY;
    }
    uint32_t legacy_count = legacy->count;
    if (legacy_count > LOG_STORE_CAPACITY) {
        legacy_count = LOG_STORE_CAPACITY;
    }
    for (uint32_t i = 0; i < count; ++i) {
        size_t legacy_index = (legacy->next_index + LOG_STORE_CAPACITY - legacy_count + i) % LOG_STORE_CAPACITY;
        persisted_log_entry_t entry = {
            .timestamp_ms = legacy->entries[legacy_index].timestamp_ms,
            .unix_time = legacy->entries[legacy_index].unix_time,
        };
        strlcpy(entry.message, legacy->entries[legacy_index].message, sizeof(entry.message));
        ring_append_internal(&entry);
    }
}

static void ensure_loaded_locked(void) {
    if (s_loaded) {
        return;
    }

    ring_reset();

    nvs_handle_t handle;
    if (open_handle(&handle) != ESP_OK) {
        s_loaded = true;
        return;
    }

    size_t size = 0;
    esp_err_t err = nvs_get_blob(handle, LOG_STORE_KEY, NULL, &size);
    if (err == ESP_OK && size > 0) {
        if (size > sizeof(s_blob_buffer)) {
            ESP_LOGW(TAG, "Log blob too large (%zu bytes), resetting", size);
        } else {
            err = nvs_get_blob(handle, LOG_STORE_KEY, s_blob_buffer, &size);
            if (err == ESP_OK && size >= sizeof(persisted_log_header_t)) {
                const persisted_log_header_t *header = (const persisted_log_header_t *)s_blob_buffer;
                if (header->version == LOG_STORE_BLOB_VERSION) {
                    uint32_t count = header->count;
                    if (count > LOG_STORE_CAPACITY) {
                        count = LOG_STORE_CAPACITY;
                    }
                    size_t expected = sizeof(persisted_log_header_t) + count * sizeof(persisted_log_entry_t);
                    if (size >= expected) {
                        const persisted_log_entry_t *entries = (const persisted_log_entry_t *)(s_blob_buffer + sizeof(persisted_log_header_t));
                        for (uint32_t i = 0; i < count; ++i) {
                            ring_append_internal(&entries[i]);
                        }
                    } else {
                        ESP_LOGW(TAG, "Truncated log blob (size=%zu expected=%zu)", size, expected);
                    }
                } else if (size == sizeof(legacy_log_ring_t)) {
                    migrate_legacy_blob((const legacy_log_ring_t *)s_blob_buffer);
                    ESP_LOGI(TAG, "Migrated legacy log blob to new format");
                } else {
                    const persisted_log_header_t *unknown = (const persisted_log_header_t *)s_blob_buffer;
                    ESP_LOGW(TAG, "Unknown log blob version 0x%08x (size=%zu), resetting", unknown->version, size);
                }
            }
        }
    }

    nvs_close(handle);
    s_loaded = true;
}

static esp_err_t erase_persisted_ring(nvs_handle_t handle) {
    esp_err_t err = nvs_erase_key(handle, LOG_STORE_KEY);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        err = ESP_OK;
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    return err;
}

static esp_err_t flush_ring_locked(void) {
    nvs_handle_t handle;
    if (open_handle(&handle) != ESP_OK) {
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;

    if (s_ring.count == 0) {
        err = erase_persisted_ring(handle);
        nvs_close(handle);
        return err;
    }

    size_t count = s_ring.count;
    size_t buffer_size = sizeof(persisted_log_header_t) + count * sizeof(persisted_log_entry_t);
    if (buffer_size > sizeof(s_blob_buffer)) {
        ESP_LOGW(TAG, "Persist buffer too small (%zu needed)", buffer_size);
        nvs_close(handle);
        return ESP_ERR_NO_MEM;
    }

    persisted_log_header_t *header = (persisted_log_header_t *)s_blob_buffer;
    header->version = LOG_STORE_BLOB_VERSION;
    header->count = count;

    persisted_log_entry_t *entries = (persisted_log_entry_t *)(s_blob_buffer + sizeof(persisted_log_header_t));
    for (size_t i = 0; i < count; ++i) {
        size_t index = (s_ring.next_index + LOG_STORE_CAPACITY - count + i) % LOG_STORE_CAPACITY;
        entries[i] = s_ring.entries[index];
    }

    err = nvs_set_blob(handle, LOG_STORE_KEY, s_blob_buffer, buffer_size);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        ESP_LOGW(TAG, "Insufficient NVS space to persist logs; clearing stored logs to preserve other data");
        ring_reset();
        erase_persisted_ring(handle);
    } else if (err == ESP_OK) {
        err = nvs_commit(handle);
        if (err == ESP_OK) {
            s_dirty_entries = 0;
        }
    } else {
        ESP_LOGW(TAG, "Failed to persist logs: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

int log_store_init(void) {
    if (s_mutex == NULL) {
        s_mutex = xSemaphoreCreateMutex();
        if (s_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create log store mutex");
            return -1;
        }
    }

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire log store mutex during init");
        return -1;
    }

    ensure_loaded_locked();
    xSemaphoreGive(s_mutex);
    return 0;
}

int log_store_append(uint64_t timestamp_ms, int64_t unix_time, const char *message) {
    if (!message) {
        return -1;
    }

    if (s_mutex == NULL && log_store_init() != 0) {
        return -1;
    }

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "Timeout acquiring log store mutex for append");
        return -1;
    }

    ensure_loaded_locked();

    persisted_log_entry_t entry = {
        .timestamp_ms = timestamp_ms,
        .unix_time = unix_time,
    };
    strlcpy(entry.message, message, sizeof(entry.message));

    ring_append_internal(&entry);
    s_dirty_entries++;

    esp_err_t err = ESP_OK;
    bool should_flush = false;
    if (s_dirty_entries >= LOG_STORE_FLUSH_THRESHOLD) {
        should_flush = true;
    } else if (s_last_flush_timestamp == 0 ||
               (timestamp_ms > s_last_flush_timestamp &&
                (timestamp_ms - s_last_flush_timestamp) >= LOG_STORE_FLUSH_INTERVAL_MS)) {
        should_flush = true;
    }

    if (should_flush) {
        err = flush_ring_locked();
        if (err == ESP_OK) {
            s_last_flush_timestamp = timestamp_ms;
        }
    }

    xSemaphoreGive(s_mutex);
    return (err == ESP_OK) ? 0 : -1;
}

size_t log_store_read(stored_log_entry_t *buffer, size_t max_entries) {
    if (!buffer || max_entries == 0) {
        return 0;
    }

    if (s_mutex == NULL && log_store_init() != 0) {
        return 0;
    }

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "Timeout acquiring log store mutex for read");
        return 0;
    }

    ensure_loaded_locked();

    size_t to_copy = s_ring.count;
    if (to_copy > max_entries) {
        to_copy = max_entries;
    }

    for (size_t i = 0; i < to_copy; i++) {
        size_t index = (s_ring.next_index + LOG_STORE_CAPACITY - s_ring.count + i) % LOG_STORE_CAPACITY;
        buffer[i].timestamp_ms = s_ring.entries[index].timestamp_ms;
        buffer[i].unix_time = s_ring.entries[index].unix_time;
        buffer[i].message = strdup(s_ring.entries[index].message);
    }

    xSemaphoreGive(s_mutex);
    return to_copy;
}

void log_store_clear(void) {
    if (s_mutex == NULL && log_store_init() != 0) {
        return;
    }

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "Timeout acquiring log store mutex for clear");
        return;
    }

    ensure_loaded_locked();
    ring_reset();
    s_dirty_entries = 0;
    s_last_flush_timestamp = 0;

    nvs_handle_t handle;
    if (open_handle(&handle) == ESP_OK) {
        esp_err_t err = nvs_erase_key(handle, LOG_STORE_KEY);
        if (err == ESP_OK) {
            nvs_commit(handle);
        }
        nvs_close(handle);
    }

    xSemaphoreGive(s_mutex);
}

void log_store_free_entries(stored_log_entry_t *entries, size_t count) {
    if (!entries) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        free(entries[i].message);
        entries[i].message = NULL;
    }
}

