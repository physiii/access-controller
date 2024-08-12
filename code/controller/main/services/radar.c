#include "radar.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "automation.h"
#include "store.h"
#include "gpio.h"
#include "cJSON.h"
#include "config_manager.h"  // Include the config manager header

// Global variables
static const char *RADAR_TAG = "radar";
radarButton_t radars[NUM_OF_RADARS];

#define BUF_SIZE            128
#define DATA_CHUNK_SIZE     10
#define ROLLING_AVG_SIZE    32
#define UART_NUM_1          UART_NUM_1  // UART for Radar 1
#define UART_NUM_2          UART_NUM_2  // UART for Radar 2
#define MOVEMENT_THRESHOLD  10          // Threshold for triggering event
#define RESET_THRESHOLD     5           // Threshold for resetting event
#define HOLD_TIME_ABOVE_THRESHOLD 25    // Number of cycles to confirm presence
#define HOLD_TIME_BELOW_THRESHOLD 100   // Number of cycles to confirm absence

void sendRadarEventToServer() {
    for (uint8_t i = 0; i < NUM_OF_RADARS; i++) {
        char state_str[300];
        char msg[600];

        snprintf(state_str, sizeof(state_str),
                 "{\"presence\":%s, \"exit\":false, \"keypad\":false, \"uptime\":1}",
                 radars[i].presence_state ? "true" : "false");
        snprintf(msg, sizeof(msg),
                 "{\"event_type\":\"load\", \"payload\":{\"services\":"
                 "[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

        addServerMessageToQueue(msg);
    }
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

        // Now copy the temporary buffer into the radar's settings
        strncpy(radars[i].settings, temp_settings, sizeof(radars[i].settings) - 1);
        radars[i].settings[sizeof(radars[i].settings) - 1] = '\0';  // Ensure null termination

        addClientMessageToQueue(radars[i].settings);
        ESP_LOGV(RADAR_TAG, "sendRadarEventToClient: %s", radars[i].settings);
    }
    return 0;
}

cJSON* createRadarStateJson() {
    cJSON *state = cJSON_CreateObject();

    // Add overall radar system state
    cJSON_AddBoolToObject(state, "systemEnabled", true);  // Assuming the system is always enabled

    // Create an array for individual radar states
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

    // Add device information to the state object
    char device_id[37];  // Assuming UUID format
    char mac_str[18];    // MAC address string (17 chars + null terminator)
    char ip_str[16];     // IP address string (15 chars + null terminator)
    char device_name[32]; // Device name string (31 chars + null terminator)

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
    sendRadarEventToServer();
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
    cJSON *settings = restoreSetting("radar_settings");
    if (settings == NULL) {
        ESP_LOGI(RADAR_TAG, "No stored radar settings found");
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

    if (above_threshold) {
        radars[radar_index].above_threshold_count++;
        radars[radar_index].below_threshold_count = 0;

        if (radars[radar_index].above_threshold_count >= HOLD_TIME_ABOVE_THRESHOLD &&
            !radars[radar_index].presence_state) {
            radars[radar_index].presence_state = true;
            radars[radar_index].movement_value = avg_movement;
            sendRadarState();
        }
    } else {
        radars[radar_index].below_threshold_count++;
        radars[radar_index].above_threshold_count = 0;

        if (radars[radar_index].below_threshold_count >= HOLD_TIME_BELOW_THRESHOLD &&
            radars[radar_index].presence_state) {
            radars[radar_index].presence_state = false;
            radars[radar_index].movement_value = avg_movement;
            sendRadarState();
        }
    }
}

void handle_radar_message(cJSON *payload) {
    if (payload == NULL) return;

    if (cJSON_GetObjectItem(payload, "getState")) {
        sendRadarState();
        return;
    }

    cJSON *channelItem = cJSON_GetObjectItem(payload, "channel");
    if (channelItem && cJSON_IsNumber(channelItem)) {
        int ch = channelItem->valueint - 1;
        if (ch < 0 || ch >= NUM_OF_RADARS) {
            ESP_LOGE(RADAR_TAG, "Invalid channel number");
            return;
        }

        cJSON *alertItem = cJSON_GetObjectItem(payload, "alert");
        if (alertItem && cJSON_IsBool(alertItem)) {
            radars[ch].alert = cJSON_IsTrue(alertItem);
        }

        cJSON *delayItem = cJSON_GetObjectItem(payload, "delay");
        if (delayItem && cJSON_IsNumber(delayItem)) {
            radars[ch].delay = delayItem->valueint;
        }

        storeRadarSettings();
        sendRadarState();
    }
}

void log_data(uint8_t *data, int len, int radar_index) {
    static uint16_t distance_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static uint8_t movement_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static int buffer_index[NUM_OF_RADARS] = {0};

    int data_index = 0;
    while (data_index + DATA_CHUNK_SIZE <= len) {
        if (data[data_index + 1] == 0xAA) {  // Position where AA occurs
            // Parse the bytes according to the protocol
            uint16_t distance_value = ((data[data_index + 3] << 8) | data[data_index + 2]) / 1000;
            uint8_t movement_value = data[data_index + 5];  // 1 byte for Movement Value
            
            // Update the rolling average buffers
            distance_buffer[radar_index][buffer_index[radar_index]] = distance_value;
            movement_buffer[radar_index][buffer_index[radar_index]] = movement_value;

            // Calculate the rolling averages
            uint32_t sum_distance = 0;
            uint32_t sum_movement = 0;
            for (int i = 0; i < ROLLING_AVG_SIZE; i++) {
                sum_distance += distance_buffer[radar_index][i];
                sum_movement += movement_buffer[radar_index][i];
            }
            uint16_t avg_distance = sum_distance / ROLLING_AVG_SIZE;  // Average distance
            uint8_t avg_movement = sum_movement / ROLLING_AVG_SIZE;   // Average movement

            // Log the averages
            if (avg_movement > 0) {
                ESP_LOGV(RADAR_TAG, "Radar %d - Avg Distance: %d, Avg Movement: %d", radar_index + 1, avg_distance, avg_movement);
            }

            // Update the buffer index
            buffer_index[radar_index] = (buffer_index[radar_index] + 1) % ROLLING_AVG_SIZE;

            // Call check_radar with the averaged movement value
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
            int len = uart_read_bytes(i == 0 ? UART_NUM_1 : UART_NUM_2, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);  // Reading every 20ms (50Hz)
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

    // Configure UART parameters
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
    xTaskCreate(radar_service, "radar_service", 5000, NULL, 10, NULL);
}
