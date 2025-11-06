#ifndef WIEGAND_REGISTRY_H
#define WIEGAND_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "cJSON.h"

#define WIEGAND_USER_CODE_MAX   64
#define WIEGAND_USER_NAME_MAX   64
#define WIEGAND_USER_ID_MAX     40
#define WIEGAND_USER_MAX_COUNT  256

typedef enum {
    WIEGAND_USER_STATUS_PENDING = 0,
    WIEGAND_USER_STATUS_ACTIVE = 1,
    WIEGAND_USER_STATUS_DISABLED = 2
} wiegand_user_status_t;

typedef struct {
    char id[WIEGAND_USER_ID_MAX];
    char code[WIEGAND_USER_CODE_MAX];
    char name[WIEGAND_USER_NAME_MAX];
    uint8_t channel;
    wiegand_user_status_t status;
    uint32_t sequence;
    uint64_t created_at_ms;
    uint64_t updated_at_ms;
} wiegand_user_t;

void wiegand_registry_init(void);
bool wiegand_registry_is_initialised(void);
size_t wiegand_registry_count(void);
const wiegand_user_t *wiegand_registry_get(size_t index);
const wiegand_user_t *wiegand_registry_find_by_code(const char *code);
const wiegand_user_t *wiegand_registry_find_by_id(const char *id);
esp_err_t wiegand_registry_add(const char *code, uint8_t channel, wiegand_user_t *out_user);
esp_err_t wiegand_registry_update_name(const char *id, const char *name);
esp_err_t wiegand_registry_update_status(const char *id, wiegand_user_status_t status);
esp_err_t wiegand_registry_remove(const char *id);
esp_err_t wiegand_registry_reload(void);
esp_err_t wiegand_registry_save(void);
cJSON *wiegand_registry_snapshot(void);

#endif // WIEGAND_REGISTRY_H

