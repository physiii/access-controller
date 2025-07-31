#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// Keypad settings
#define KEYPAD_MCP_IO_1 A3
#define KEYPAD_MCP_IO_2 B3
#define NUM_OF_KEYPADS 2

typedef struct {
    int pin;
    bool alert;
    bool isPressed;
    bool prevPress;
    int count;
    bool expired;
    bool enable;
    int delay;
    int channel;
    char settings[1000];
    char key[50];
    char type[40];
    cJSON *payload;
} keypadButton_t;

extern char keypad_service_message[2000];
extern bool keypad_service_message_ready;
extern cJSON *keypad_payload;
extern keypadButton_t keypads[NUM_OF_KEYPADS];

void start_keypad_timer(keypadButton_t *pad, bool val);
void check_keypad_timer(keypadButton_t *pad);
void check_keypads(keypadButton_t *pad);
void alertOnKeypad(int ch, bool val);
void enableKeypad(int ch, bool val);
void setKeypadArmDelay(int ch, int val);
int storeKeypadSettings(void);
int restoreKeypadSettings(void);
int load_keypad_state_from_flash(void);
void handle_keypad_message(cJSON *payload);
void keypad_main(void);

#endif // KEYPAD_H
