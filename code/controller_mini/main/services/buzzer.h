#ifndef BUZZER_H
#define BUZZER_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

// Structure representing the buzzer state
struct buzzer {
    uint8_t pin;
    uint8_t beepCount;
    uint8_t longBeepCount;
    bool enable;
    bool contactAlert;
};

// Global buzzer instance
extern struct buzzer bzr;

// Buzzer functions
void startBeep(struct buzzer *bz);
void startLongBeep(struct buzzer *bz);
void beep(int cnt);
void longBeep(int cnt);
void turn_buzzer_on(bool val);
void enable_buzzer(bool val);
void buzzer_main(void);

#endif // BUZZER_H
