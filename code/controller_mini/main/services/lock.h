#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>
#include <stdbool.h>
#include "cJSON.h"

// Lock settings
#define LOCK_MCP_IO_1       A0
#define LOCK_MCP_IO_2       B0
#define NUM_OF_LOCKS        2

// Lock signal and contact pins
extern const uint8_t LOCK_SIGNAL_PIN_1;
extern const uint8_t LOCK_SIGNAL_PIN_2;
extern const uint8_t LOCK_CONTACT_PIN_1;
extern const uint8_t LOCK_CONTACT_PIN_2;

// Lock structure
typedef struct {
    uint8_t controlPin;
    uint8_t contactPin;
    uint8_t signalPin;
    uint8_t channel;
    bool isContact;
    bool prevIsContact;
    bool shouldLock;
    bool isLocked;
    bool isSignal;
    bool pulse;
    bool polarity;
    bool expired;
    bool enable;
    bool enableContactAlert;
    int delay;
    int count;
    bool alert;
    bool sentSignalAlert;
    bool sentContactAlert;
    cJSON *payload;
    char settings[200];
    char key[50];
    char type[40];
} Lock;

// Global variables
extern bool ARM;
extern bool ALERT;
extern char lock_service_message[1000];
extern bool lock_service_message_ready;
extern int relock_delay;
extern int button_disabled;
extern cJSON *lock_payload;
extern int LOCK_DEBOUNCE_DELAY;
extern bool lock_contact_timer_1_expired;
extern int lock_count;
extern Lock locks[NUM_OF_LOCKS];

// Lock functions
void start_lock_contact_timer(Lock *lck, bool val);
void check_lock_contact_timer(Lock *lck);
void createLockServiceMessage(bool value);
void enableLock(int ch, bool val);
void arm_lock(int channel, bool arm, bool alert);
int storeLockSettings(void);
int restoreLockSettings(void);
int sendLockState(void);
void handle_lock_message(cJSON *payload);
void lock_main(void);

#endif // LOCK_H
