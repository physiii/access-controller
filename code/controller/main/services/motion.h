#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// Motion sensor settings
#define MOTION_IO_1  A6
#define MOTION_IO_2  B6
#define NUM_OF_MOTIONS  2
#define MOMENTARY  1

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
    cJSON *payload;
    char settings[1000];
    char key[50];
    char type[40];
} motion_t;

extern char motion_service_message[2000];
extern bool motion_service_message_ready;
extern cJSON *motion_payload;
extern bool MOTION_ALERT;
extern motion_t motions[NUM_OF_MOTIONS];

void start_motion_timer(motion_t *mtn, bool val);
void check_motion_timer(motion_t *mtn);
void check_motions(motion_t *mtn);
void alertOnMotion(int ch, bool val);
int storeMotionSettings(void);
int restoreMotionSettings(void);
int sendMotionState(void);
void handle_motion_message(cJSON *payload);
void motion_main(void);

#endif // MOTION_H
