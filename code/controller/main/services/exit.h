#ifndef EXIT_H
#define EXIT_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// Exit button settings
#define EXIT_BUTTON_MCP_IO_1         A5
#define EXIT_BUTTON_MCP_IO_2         B5
#define NUM_OF_EXITS                 2

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
} exitButton_t;

extern char exit_service_message[2000];
extern bool exit_service_message_ready;
extern cJSON *exit_payload;
extern exitButton_t exits[NUM_OF_EXITS];

void start_exit_timer(exitButton_t *ext, bool val);
void check_exit_timer(exitButton_t *ext);
void enableExit(int ch, bool val);
void alertOnExit(int ch, bool val);
void setArmDelay(int ch, int val);
void check_exit(exitButton_t *ext);
void handle_exit_message(cJSON *payload);
void storeExitSettings(void);
void restoreExitSettings(void);
int sendExitState(void);
int handle_exit_property(char *prop);
void exit_main(void);

#endif // EXIT_H
