#ifndef AUTHORIZE_H
#define AUTHORIZE_H

#include "cJSON.h"
#include <stdbool.h>

// Constants
#define MAX_UIDS        400
#define MAX_UID_SIZE    50
#define MAX_USERS       400
#define MAX_NAME_SIZE   50
#define MAX_PIN_SIZE    10
#define MAX_SSID_SIZE   32
#define MAX_PASS_SIZE   64

// User info structure
typedef struct {
    char uid[MAX_UID_SIZE];
    char name[MAX_NAME_SIZE];
    char pin[MAX_PIN_SIZE];
} user_info;

// Global variables
extern int CHECK_UID;
extern int ADD_UID;
extern int REMOVE_UID;

extern char uids[MAX_UIDS][MAX_UID_SIZE];
extern char uids_tmp[MAX_UIDS][MAX_UID_SIZE];

extern int current_mode;
extern char auth_service_message[1000];
extern bool auth_service_message_ready;
extern cJSON *auth_uids;
extern cJSON *new_auth_uids;
extern cJSON *auth_users;

// Functions
void addUser(char *uuid, char *name, char *pin);
int is_pin_authorized(const char *incomingPin);
void send_user_count(void);
void sendInfo(void);
void handle_authorize_message(cJSON *payload);
void auth_main(void);

#endif // AUTHORIZE_H
