/**
 * Simple registry for 433MHz remote fobs (RF codes).
 * Supports registration mode, listing, renaming, and deleting stored codes.
 */
#ifndef RF_REGISTRY_H
#define RF_REGISTRY_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "cJSON.h"

void rf_registry_init(void);
bool rf_registry_is_active(void);
esp_err_t rf_registration_start(void);
esp_err_t rf_registration_stop(void);
void rf_registry_on_code(uint32_t code, size_t pulse_count);
esp_err_t rf_registry_update_name(const char *id, const char *name);
esp_err_t rf_registry_remove(const char *id);
cJSON *rf_state_snapshot(void);
bool rf_registry_handle_code(uint32_t code);
esp_err_t rf_registry_update_config(const char *id, const char *mode, int channel_mask, int exit_seconds, bool alert);

#endif /* RF_REGISTRY_H */
