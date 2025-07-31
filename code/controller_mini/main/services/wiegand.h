#ifndef WIEGAND_H
#define WIEGAND_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define WG_DELAY            80
#define NUM_OF_WIEGANDS     2
#define NUM_OF_KEYS         12
#define KEYCODE_LENGTH      6

typedef struct {
    uint32_t gpio_num;
    int wg_index;
    int gpio_val;
} gpio_event_t;

typedef struct {
    uint8_t pin0;
    uint8_t pin1;
    uint8_t pin_push;
    char code[25];
    uint8_t bitCount;
    char name[100];
    int count;
    int keypressCount;
    bool expired;
    bool keypressExpired;
    bool enable;
    bool newKey;
    bool newCode;
    char incomingCode[50];
    char fingerCode[50];
    uint8_t incomingCodeCount;
    int keyCount;
    bool alert;
    int delay;
    int keypressTimeout;
    int channel;
} wiegand_t;

extern wiegand_t wg[NUM_OF_WIEGANDS];
extern QueueHandle_t gpio_evt_queue;

void beep_keypad(int count, int ch);
void start_keypress_timer(wiegand_t *wg, bool val);
void check_keypress_timer(wiegand_t *wg);
void start_wiegand_timer(wiegand_t *wg, bool val);
void check_wiegand_timer(wiegand_t *wg);
void handleKeyCode(wiegand_t *wg);
void enableWiegand(int ch, bool val);
void wiegand_main(void);

#endif // WIEGAND_H
