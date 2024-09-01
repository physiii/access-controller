#include "radar.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "automation.h"
#include "store.h"
#include "gpio.h"
#include "cJSON.h"
#include "config_manager.h"

// Global variables
static const char *RADAR_TAG = "radar";
radarButton_t radars[NUM_OF_RADARS];

#define BUF_SIZE            128
#define DATA_CHUNK_SIZE     10
#define ROLLING_AVG_SIZE    32
#define UART_NUM_1          UART_NUM_1
#define UART_NUM_2          UART_NUM_2
#define MOVEMENT_THRESHOLD  10
#define RESET_THRESHOLD     5
#define HOLD_TIME_ABOVE_THRESHOLD 25
#define HOLD_TIME_BELOW_THRESHOLD 100

// Increased buffer sizes
#define STATE_STR_SIZE      512
#define EVENT_STR_SIZE      1024
#define MSG_SIZE            2048

void sendRadarEventToServer(uint8_t radar_index) {
    if (radar_index >= NUM_OF_RADARS) {
        ESP_LOGE(RADAR_TAG, "Invalid radar index");
        return;
    }

    char state_str[STATE_STR_SIZE];
    char event_str[EVENT_STR_SIZE];
    char msg[MSG_SIZE];

    snprintf(state_str, sizeof(state_str),
             "{\"presence\":%s, \"exit\":false, \"keypad\":false, \"uptime\":1}",
             radars[radar_index].presence_state ? "true" : "false");

    snprintf(event_str, sizeof(event_str),
             "\"event\":{\"description\":\"%s\", \"location\":\"Radar %d\", \"movement_value\":%d, \"distance_value\":%d}",
             radars[radar_index].event_description, radar_index + 1, 
             radars[radar_index].last_avg_movement, radars[radar_index].last_avg_distance);

    snprintf(msg, sizeof(msg),
             "{\"event_type\":\"load\", \"payload\":{\"services\":"
             "[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s, %s}]}}",
             state_str, event_str);

    addServerMessageToQueue(msg);
}

int sendRadarEventToClient() {
    char temp_settings[sizeof(radars[0].settings)];  // Temporary buffer

    for (uint8_t i = 0; i < NUM_OF_RADARS; i++) {
        snprintf(temp_settings, sizeof(temp_settings),
                 "{\"eventType\":\"%s\", "
                 "\"payload\":{\"channel\":%d, \"alert\": %s, \"delay\": %d, \"presence\": %s}}",
                 radars[i].type,
                 i + 1,
                 radars[i].alert ? "true" : "false",
                 radars[i].delay,
                 radars[i].presence_state ? "true" : "false");

        strncpy(radars[i].settings, temp_settings, sizeof(radars[i].settings) - 1);
        radars[i].settings[sizeof(radars[i].settings) - 1] = '\0';  // Ensure null termination

        addClientMessageToQueue(radars[i].settings);
        ESP_LOGV(RADAR_TAG, "sendRadarEventToClient: %s", radars[i].settings);
    }
    return 0;
}

cJSON* createRadarStateJson() {
    cJSON *state = cJSON_CreateObject();
    cJSON_AddBoolToObject(state, "systemEnabled", true);
    cJSON *radarArray = cJSON_AddArrayToObject(state, "radars");

    for (int i = 0; i < NUM_OF_RADARS; i++) {
        cJSON *radarObject = cJSON_CreateObject();
        cJSON_AddNumberToObject(radarObject, "channel", radars[i].channel);
        cJSON_AddBoolToObject(radarObject, "alert", radars[i].alert);
        cJSON_AddNumberToObject(radarObject, "delay", radars[i].delay);
        cJSON_AddNumberToObject(radarObject, "movementValue", radars[i].movement_value);
        cJSON_AddBoolToObject(radarObject, "presence", radars[i].presence_state);
        
        cJSON_AddItemToArray(radarArray, radarObject);
    }

    char device_id[37];
    char mac_str[18];
    char ip_str[16];
    char device_name[32];

    get_device_id(device_id, sizeof(device_id));
    get_mac_address(mac_str, sizeof(mac_str));
    get_ip_address(ip_str, sizeof(ip_str));
    get_device_name(device_name, sizeof(device_name));

    cJSON_AddStringToObject(state, "device_id", device_id);
    cJSON_AddStringToObject(state, "mac_address", mac_str);
    cJSON_AddStringToObject(state, "ip_address", ip_str);
    cJSON_AddStringToObject(state, "device_name", device_name);

    return state;
}

void sendRadarState() {
    sendRadarEventToClient();
}

int storeRadarSettings() {
    cJSON *settings = cJSON_CreateObject();
    cJSON *radarArray = cJSON_AddArrayToObject(settings, "radars");

    for (uint8_t i = 0; i < NUM_OF_RADARS; i++) {
        cJSON *radarObject = cJSON_CreateObject();
        cJSON_AddNumberToObject(radarObject, "channel", radars[i].channel);
        cJSON_AddBoolToObject(radarObject, "alert", radars[i].alert);
        cJSON_AddNumberToObject(radarObject, "delay", radars[i].delay);
        cJSON_AddItemToArray(radarArray, radarObject);
    }

    storeSetting("radar_settings", settings);
    cJSON_Delete(settings);
    return 0;
}

int restoreRadarSettings() {
    cJSON *settings = cJSON_Parse(restoreSetting("radar_settings"));
    if (settings == NULL) {
        ESP_LOGI(RADAR_TAG, "No stored radar settings found or invalid JSON");
        return -1;
    }

    cJSON *radarArray = cJSON_GetObjectItemCaseSensitive(settings, "radars");
    if (!cJSON_IsArray(radarArray)) {
        ESP_LOGE(RADAR_TAG, "Invalid radar settings format");
        cJSON_Delete(settings);
        return -1;
    }

    int i = 0;
    cJSON *radarObject = NULL;
    cJSON_ArrayForEach(radarObject, radarArray) {
        if (i >= NUM_OF_RADARS) break;

        cJSON *channel = cJSON_GetObjectItemCaseSensitive(radarObject, "channel");
        cJSON *alert = cJSON_GetObjectItemCaseSensitive(radarObject, "alert");
        cJSON *delay = cJSON_GetObjectItemCaseSensitive(radarObject, "delay");

        if (cJSON_IsNumber(channel)) radars[i].channel = channel->valueint;
        if (cJSON_IsBool(alert)) radars[i].alert = cJSON_IsTrue(alert);
        if (cJSON_IsNumber(delay)) radars[i].delay = delay->valueint;

        i++;
    }

    cJSON_Delete(settings);
    return 0;
}

void check_radar(int radar_index, uint8_t avg_movement) {
    bool above_threshold = (avg_movement > MOVEMENT_THRESHOLD);
    radarButton_t *radar = &radars[radar_index];

    if (above_threshold) {
        radar->above_threshold_count++;
        radar->below_threshold_count = 0;

        if (radar->above_threshold_count >= HOLD_TIME_ABOVE_THRESHOLD &&
            !radar->presence_state) {
            radar->presence_state = true;
            radar->movement_value = avg_movement;
            snprintf(radar->event_description, sizeof(radar->event_description),
                     "Sustained Movement Detected. Movement Value: %d", avg_movement);
            sendRadarEventToServer(radar_index);
            sendRadarEventToClient();
        }
    } else {
        radar->below_threshold_count++;
        radar->above_threshold_count = 0;

        if (radar->below_threshold_count >= HOLD_TIME_BELOW_THRESHOLD &&
            radar->presence_state) {
            radar->presence_state = false;
            radar->movement_value = avg_movement;
            snprintf(radar->event_description, sizeof(radar->event_description),
                     "No Significant Movement Detected. Movement Value: %d", avg_movement);
            sendRadarEventToServer(radar_index);
            sendRadarEventToClient();
        }
    }

    // Event for intermittent movement
    if (avg_movement > MOVEMENT_THRESHOLD && avg_movement < RESET_THRESHOLD) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Intermittent Movement Detected. Movement Value: %d", avg_movement);
    }

    // Events based on movement speed
    if (avg_movement > 50) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Rapid Movement Detected. Movement Value: %d", avg_movement);
    } else if (avg_movement > 0 && avg_movement <= 10) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Slow Movement Detected. Movement Value: %d", avg_movement);
    }

    // Distance-based descriptions
    if (radar->last_avg_distance < 100) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Movement Detected at Close Range. Distance: %d mm", radar->last_avg_distance);
    } else if (radar->last_avg_distance < 500) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Movement Detected at Medium Range. Distance: %d mm", radar->last_avg_distance);
    } else {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Movement Detected at Long Range. Distance: %d mm", radar->last_avg_distance);
    }

    // Presence re-confirmation
    if (radar->presence_state && radar->below_threshold_count == 1) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "Presence Confirmed After Absence. Movement Value: %d", avg_movement);
    }

    // No presence after prolonged absence
    if (!radar->presence_state && radar->below_threshold_count == HOLD_TIME_BELOW_THRESHOLD) {
        snprintf(radar->event_description, sizeof(radar->event_description),
                 "No Presence Confirmed After Extended Absence. Movement Value: %d", avg_movement);
    }
}

void log_data(uint8_t *data, int len, int radar_index) {
    static uint16_t distance_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static uint8_t movement_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static int buffer_index[NUM_OF_RADARS] = {0};

    int data_index = 0;
    while (data_index + DATA_CHUNK_SIZE <= len) {
        if (data[data_index + 1] == 0xAA) {  // Position where AA occurs
            uint16_t distance_value = ((data[data_index + 3] << 8) | data[data_index + 2]) / 1000;
            uint8_t movement_value = data[data_index + 5];  // 1 byte for Movement Value
            
            distance_buffer[radar_index][buffer_index[radar_index]] = distance_value;
            movement_buffer[radar_index][buffer_index[radar_index]] = movement_value;

            uint32_t sum_distance = 0;
            uint32_t sum_movement = 0;
            for (int i = 0; i < ROLLING_AVG_SIZE; i++) {
                sum_distance += distance_buffer[radar_index][i];
                sum_movement += movement_buffer[radar_index][i];
            }
            uint16_t avg_distance = sum_distance / ROLLING_AVG_SIZE;
            uint8_t avg_movement = sum_movement / ROLLING_AVG_SIZE;

            radars[radar_index].last_avg_distance = avg_distance;
            radars[radar_index].last_avg_movement = avg_movement;

            if (avg_movement > 0) {
                ESP_LOGV(RADAR_TAG, "Radar %d - Avg Distance: %d, Avg Movement: %d", radar_index + 1, avg_distance, avg_movement);
            }

            buffer_index[radar_index] = (buffer_index[radar_index] + 1) % ROLLING_AVG_SIZE;

            check_radar(radar_index, avg_movement);
            
            data_index += DATA_CHUNK_SIZE;
        } else {
            data_index++;
        }
    }
}

static void radar_service(void *pvParameter) {
    uint8_t data[BUF_SIZE];
    while (1) {
        for (int i = 0; i < NUM_OF_RADARS; i++) {
            int len = uart_read_bytes(i == 0 ? UART_NUM_1 : UART_NUM_2, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);
            if (len > 0) {
                log_data(data, len, i);
            }
        }
        handle_radar_message(checkServiceMessage("radar"));
        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }
}

void init_uart(int uart_num, int uart_pin_tx, int uart_pin_rx) {
    const uart_config_t uart_config = {
        .baud_rate = 256000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, uart_pin_tx, uart_pin_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
}

void radar_main(int uart_pin_tx_1, int uart_pin_rx_1, int uart_pin_tx_2, int uart_pin_rx_2) {
    ESP_LOGI(RADAR_TAG, "Initializing Radar Service %d %d", uart_pin_tx_1, uart_pin_rx_1);
    ESP_LOGI(RADAR_TAG, "Initializing Radar Service %d %d", uart_pin_tx_2, uart_pin_rx_2);

    // Initialize UART for both radars
    init_uart(UART_NUM_1, uart_pin_tx_1, uart_pin_rx_1);
    init_uart(UART_NUM_2, uart_pin_tx_2, uart_pin_rx_2);

    // Restore radar settings
    restoreRadarSettings();

    // Set GPIO directions
    gpio_set_direction(radars[0].pin, GPIO_MODE_INPUT);
    gpio_set_direction(radars[1].pin, GPIO_MODE_INPUT);

    // Create radar service task
    xTaskCreate(radar_service, "radar_service", 8 * 1024, NULL, 10, NULL);
}

void handle_radar_message(cJSON *payload) {
    if (payload == NULL) {
        return;
    }

    cJSON *channel = cJSON_GetObjectItemCaseSensitive(payload, "channel");
    cJSON *alert = cJSON_GetObjectItemCaseSensitive(payload, "alert");
    cJSON *delay = cJSON_GetObjectItemCaseSensitive(payload, "delay");

    if (cJSON_IsNumber(channel) && channel->valueint >= 1 && channel->valueint <= NUM_OF_RADARS) {
        int index = channel->valueint - 1;
        
        if (cJSON_IsBool(alert)) {
            radars[index].alert = cJSON_IsTrue(alert);
        }
        
        if (cJSON_IsNumber(delay)) {
            radars[index].delay = delay->valueint;
        }

        // Store the updated settings
        storeRadarSettings();

        // Send the updated state to the client
        sendRadarEventToClient();
    }

    cJSON_Delete(payload);
}

void alertOnRadar(int ch, bool val) {
    if (ch >= 1 && ch <= NUM_OF_RADARS) {
        int index = ch - 1;
        radars[index].alert = val;
        sendRadarEventToClient();
    }
}