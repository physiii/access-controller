#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "cJSON.h"

#define SERVICE_LOOP 100
#define MAX_QUEUE_SIZE 20  // Conservative size
#define MAX_MESSAGE_SIZE 2000

// Essential macros
#define STRIKE 0
#if STRIKE
#define USE_MCP23017 0
#else
#define USE_MCP23017 1
#endif

// Global variable declarations (extern)
extern char token[700];
extern char device_id[100];
extern char server_ip[32];
extern char server_port[8];

struct ServiceMessage {
    cJSON *messageQueue[MAX_QUEUE_SIZE];
    uint8_t queueCount;
    SemaphoreHandle_t mutex;
};

extern struct ServiceMessage serviceMessage;
extern struct ServiceMessage clientMessage;

void checkServiceMessage(void);
cJSON *checkServiceMessageByType(char *eventType);
cJSON *checkServiceMessageByAction(char *id, char *action);
cJSON *checkServiceMessageByKey(char *key);
void addServiceMessageToQueue(cJSON *message);
void addClientMessageToQueue(cJSON *message);
void addServerMessageToQueue(const char *message);
void init_automation_queues(void);


#endif // AUTOMATION_H
