#ifndef RADAR_H
#define RADAR_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// Radar settings
#define RADAR_BUTTON_MCP_IO_1 A3
#define RADAR_BUTTON_MCP_IO_2 B3
#define NUM_OF_RADARS 2  // Support for two radar sensors

typedef struct {
    int pin;
    bool alert;
    int movement_value;  // Field to store movement value
    int delay;
    int channel;
    char settings[1000];
    char key[50];
    char type[40];
    cJSON *payload;
} radarButton_t;

// Global variables
extern char radar_service_message[2000];
extern bool radar_service_message_ready;
extern cJSON *radar_payload;
extern radarButton_t radars[NUM_OF_RADARS];

// Radar functions
void alertOnRadar(int ch, bool val);
void check_radar(int radar_index, uint8_t avg_movement);  // Updated to match implementation
void handle_radar_message(cJSON *payload);
int storeRadarSettings(void);
int restoreRadarSettings(void);
int sendRadarEventToClient(void);  // Corrected function signature
void radar_main(int uart_pin_tx_1, int uart_pin_rx_1, int uart_pin_tx_2, int uart_pin_rx_2);
void init_uart(int uart_num, int uart_pin_tx, int uart_pin_rx);

#endif // RADAR_H
