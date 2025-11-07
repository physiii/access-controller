#ifndef LOG_STORE_H
#define LOG_STORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t timestamp_ms;
    int64_t unix_time;
    char *message;
} stored_log_entry_t;

#define LOG_STORE_CAPACITY 100
#define LOG_STORE_MESSAGE_MAX 96

int log_store_init(void);
int log_store_append(uint64_t timestamp_ms, int64_t unix_time, const char *message);
size_t log_store_read(stored_log_entry_t *buffer, size_t max_entries);
void log_store_free_entries(stored_log_entry_t *entries, size_t count);
void log_store_clear(void);
int log_store_flush_now(void);

#ifdef __cplusplus
}
#endif

#endif
