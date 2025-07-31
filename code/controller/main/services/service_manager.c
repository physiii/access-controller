// services/service_manager.c

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "drivers/i2c.c"
#include "drivers/mcp23x17.h"
#include "authorize.h"
#include "buzzer.h"
#include "lock.h"
#include "wiegand.h"
#include "exit.h"
#include "radar.h"
#include "motion.h"
#include "radar.h"
#include "gpio.h"
#include "keypad.h"
#include "fob.h"
#include "motion.h"
#include "server.h"
#include "automation.h"
#include "config_manager.h"

static const char *SVC_TAG = "ServiceManager";

// Initializers for each service
void gpio_start() {
    ESP_LOGI(SVC_TAG, "Initializing GPIO");
    gpio_main();
}

void i2c_start() {
    ESP_LOGI(SVC_TAG, "Initializing I2C");
    i2c_main();  // Use the existing I2C initialization from drivers/i2c.c
}

void mcp23x17_start() {
    ESP_LOGI(SVC_TAG, "Initializing MCP23X17");
    mcp23x17_main();  // Initialize MCP23X17 on I2C bus 1
}

void auth_start() {
    ESP_LOGI(SVC_TAG, "Initializing Authorization Module");
    auth_main();  // Specific initialization for authorization
}

void buzzer_start() {
    ESP_LOGI(SVC_TAG, "Initializing Buzzer");
    buzzer_main();  // Buzzer connected to GPIO 4
}

void wiegand_start() {
    ESP_LOGI(SVC_TAG, "Initializing Wiegand Protocol Interface");
    wiegand_main();  // Data0 and Data1 pins
}

void exit_start() {
    ESP_LOGI(SVC_TAG, "Initializing Exit Button System");
    exit_main();  // Exit button connected to GPIO 16
}

void lock_start() {
    ESP_LOGI(SVC_TAG, "Initializing Lock Control");
    lock_main();  // Lock control connected to GPIO 25
}

void fob_start() {
    ESP_LOGI(SVC_TAG, "Initializing Fob Reader");
    fob_main();  // Fob reader on I2C bus 1
}

void motion_start() {
    ESP_LOGI(SVC_TAG, "Initializing Motion Detection");
    motion_main();  // Motion sensor on GPIO 27
}

void radar_start() {
    ESP_LOGI(SVC_TAG, "Initializing Radar Sensor");
    radar_main(4, 5, 8 ,9);  // Radar sensor on GPIO 17
}

void keypad_start() {
    ESP_LOGI(SVC_TAG, "Initializing Keypad");
    keypad_main();
}

void server_start() {
    ESP_LOGI(SVC_TAG, "Initializing Server Connections");
    server_main();
}

// Function to initialize all services
void service_manager_start_all() {
    ESP_LOGI(SVC_TAG, "Starting all services...");
    gpio_start();
    i2c_start();
    mcp23x17_start();
    auth_start();
    buzzer_start();
    wiegand_start();
    exit_start();
    lock_start();
    fob_start();
    motion_start();
    radar_start();
    keypad_start();
    server_start();
    config_manager_start();
    ESP_LOGI(SVC_TAG, "All services started successfully.");
}
