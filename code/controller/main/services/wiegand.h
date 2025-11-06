#ifndef WIEGAND_SERVICE_H
#define WIEGAND_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "cJSON.h"

void wiegand_main(void);

esp_err_t wiegand_registration_start(uint8_t channel);
esp_err_t wiegand_registration_stop(bool promote_pending);
bool wiegand_registration_is_active(void);
uint8_t wiegand_registration_channel(void);
const char *wiegand_registration_last_duplicate(void);
size_t wiegand_registration_pending_count(void);
cJSON *wiegand_state_snapshot(void);

#endif // WIEGAND_SERVICE_H

