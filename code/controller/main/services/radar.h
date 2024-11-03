// radar.h

#ifndef RADAR_H
#define RADAR_H

#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

// Radar settings
#define RADAR_BUTTON_MCP_IO_1 A3
#define RADAR_BUTTON_MCP_IO_2 B3
#define NUM_OF_RADARS 1
#define MAX_EVENT_DESCRIPTION_LENGTH 2048  // Increased size to accommodate larger event descriptions

typedef struct {
    int pin;
    bool alert;
    int movement_value;        // Field to store movement value
    int delay;
    int channel;
    bool presence_state;       // Field to store the presence state
    int above_threshold_count; // Counter for holding above threshold
    int below_threshold_count; // Counter for holding below threshold
    char settings[1000];
    char key[50];
    char type[40];
    cJSON *payload;
    char event_description[MAX_EVENT_DESCRIPTION_LENGTH]; // Increased buffer size for event description
    int last_avg_movement;
    int last_avg_distance;
    char room_name[50];

    // Fields for movement calculation
    uint16_t previous_distance;
    uint32_t previous_time_ms; // Time in milliseconds
    bool has_previous;
} radarButton_t;

// Global variables
extern char radar_service_message[2000];
extern bool radar_service_message_ready;
extern cJSON *radar_payload;
extern radarButton_t radars[NUM_OF_RADARS];

// Radar functions
void alertOnRadar(int ch, bool val);
void check_radar(int radar_index, uint8_t avg_movement);
void handle_radar_message(cJSON *payload);
int storeRadarSettings(void);
int restoreRadarSettings(void);
int sendRadarEventToClient(void);
void radar_main(int uart_pin_tx_1, int uart_pin_rx_1, int uart_pin_tx_2, int uart_pin_rx_2);
void init_uart(int uart_num, int uart_pin_tx, int uart_pin_rx);

#endif // RADAR_H
