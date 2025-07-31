#ifndef FOB_H
#define FOB_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// FOB settings
#define FOB_IO_1  A7
#define FOB_IO_2  B7
#define NUM_OF_FOBS  2
#define MOMENTARY  1

typedef struct {
    int pin;
    bool alert;
    bool isPressed;
    bool prevPress;
    int count;
    bool expired;
    bool enable;
    bool latch;  // New field for latch mode (false = momentary, true = latch)
    int delay;
    int channel;
    cJSON *payload;
    char settings[1000];
    char key[50];
    char type[40];
} fob_t;

extern char fob_service_message[2000];
extern bool fob_service_message_ready;
extern cJSON *fob_payload;
extern bool FOB_ALERT;
extern fob_t fobs[NUM_OF_FOBS];

void start_fob_timer(fob_t *fb, bool val);
void check_fob_timer(fob_t *fb);
void check_fobs(fob_t *fb);
void alertOnFob(int ch, bool val);
void storeFobSettings(void);
void restoreFobSettings(void);
int sendFobState(void);
void handle_fob_message(cJSON *payload);
void fob_main(void);

#endif // FOB_H
