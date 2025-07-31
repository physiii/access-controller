#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <string.h>
#include <ctype.h>
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"

// Function declarations
void beep_keypad(int, int);
void set_mcp_io(uint8_t, bool);
bool get_mcp_io(uint8_t);

// Macros
#define SERVICE_LOOP 100
#define SERVICE_LOOP_SHORT 10
#define STRIKE 0
#define MAX_QUEUE_SIZE 20

#if STRIKE
#define USE_MCP23017 0
#else
#define USE_MCP23017 1
#endif

#define NO_DATA_TIMEOUT_SEC 5

// Global variable declarations (extern)
extern char wss_data_in[1800];
extern char wss_data_out[1800];
extern bool wss_data_out_ready;

extern bool run_relay;
extern bool get_time;

extern char token[700];
extern char device_id[100];
extern char wifi_ssid[32];
extern char wifi_password[64];
extern char server_ip[32];
extern char server_port[8];

extern bool disconnect_from_relay;
extern bool connect_to_relay;

extern const char *TAG;

struct ServiceMessage
{
    cJSON *message;
    cJSON *messageQueue[MAX_QUEUE_SIZE];
    bool read;
    int timeout;
    int queueCount;
};

struct ClientMessage
{
    char message[1000];
    char messageQueue[MAX_QUEUE_SIZE][1000];
    bool readyToSend;
    int timeout;
    int queueCount;
};

struct ServerMessage
{
    char message[1000];
    char messageQueue[MAX_QUEUE_SIZE][1000];
    bool readyToSend;
    int timeout;
    int queueCount;
};

extern struct ServiceMessage serviceMessage;
extern struct ClientMessage clientMessage;
extern struct ServerMessage serverMessage;

// Function declarations
cJSON *checkServiceMessage(char *eventType);
cJSON *checkServiceMessageByAction(char *id, char *action);
cJSON *checkServiceMessageByKey(char *key);
void addServiceMessageToQueue(cJSON *message);
void addServerMessageToQueue(const char *message);
void addClientMessageToQueue(const char *message);
void serviceMessageTask(void *pvParameter);
void serverMessageTask(void *pvParameter);
void clientMessageTask(void *pvParameter);

#endif // AUTOMATION_H
