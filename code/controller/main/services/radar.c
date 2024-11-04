// services/radar.c

#include "radar.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "automation.h"
#include "store.h"
#include "gpio.h"
#include "cJSON.h"
#include "config_manager.h"
#include "mqtt_client.h"
#include "esp_timer.h" // For high-resolution timers

// Global variables
static const char *RADAR_TAG = "radar";
radarButton_t radars[NUM_OF_RADARS];

#define BUF_SIZE                   128
#define DATA_CHUNK_SIZE            10
#define ROLLING_AVG_SIZE           32
#define MOVEMENT_THRESHOLD         0
#define RESET_THRESHOLD            5
#define HOLD_TIME_ABOVE_THRESHOLD  5
#define HOLD_TIME_BELOW_THRESHOLD  10
#define MSG_SIZE                   2048

// MQTT client handle
static esp_mqtt_client_handle_t mqtt_client = NULL;

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

int restoreRadarSettings(void) {
    const char *json_str = get_char("radar_settings");
    if (!json_str || strcmp(json_str, "") == 0) {
        ESP_LOGI(RADAR_TAG, "No stored radar settings found.");
        // Do not free json_str as it's not dynamically allocated
        return -1;
    }

    cJSON *settings = cJSON_Parse(json_str);
    if (!settings) {
        ESP_LOGE(RADAR_TAG, "Failed to parse radar settings JSON.");
        return -1;
    }

    cJSON *radarArray = cJSON_GetObjectItemCaseSensitive(settings, "radars");
    if (!cJSON_IsArray(radarArray)) {
        ESP_LOGE(RADAR_TAG, "Invalid radar settings format.");
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

int storeRadarSettings(void) {
    cJSON *settings = cJSON_CreateObject();
    if (!settings) {
        ESP_LOGE(RADAR_TAG, "Failed to create JSON object for settings.");
        return -1;
    }

    cJSON *radarArray = cJSON_AddArrayToObject(settings, "radars");
    if (!radarArray) {
        ESP_LOGE(RADAR_TAG, "Failed to create JSON array for radars.");
        cJSON_Delete(settings);
        return -1;
    }

    for (uint8_t i = 0; i < NUM_OF_RADARS; i++) {
        cJSON *radarObject = cJSON_CreateObject();
        if (!radarObject) {
            ESP_LOGE(RADAR_TAG, "Failed to create JSON object for radar %d.", i);
            continue;
        }
        cJSON_AddNumberToObject(radarObject, "channel", radars[i].channel);
        cJSON_AddBoolToObject(radarObject, "alert", radars[i].alert);
        cJSON_AddNumberToObject(radarObject, "delay", radars[i].delay);
        cJSON_AddItemToArray(radarArray, radarObject);
    }

    char *json_str = cJSON_PrintUnformatted(settings);
    if (json_str) {
        store_char("radar_settings", json_str);
        ESP_LOGI(RADAR_TAG, "Stored radar settings: %s", json_str);
        free(json_str);
    } else {
        ESP_LOGE(RADAR_TAG, "Failed to print radar settings to JSON string.");
    }
    cJSON_Delete(settings);
    return 0;
}

cJSON* createRadarStateJson(void) {
    cJSON *state = cJSON_CreateObject();
    if (!state) {
        ESP_LOGE(RADAR_TAG, "Failed to create JSON object for state.");
        return NULL;
    }

    // Add device information in the specified format
    cJSON_AddStringToObject(state, "device_type", "radar");

    // Set up the room name
    char room_name[50];
    get_room_name(room_name, sizeof(room_name));
    cJSON_AddStringToObject(state, "room", room_name);

    // Populate device fields
    char device_id[37], mac_str[18], ip_str[16], device_name[32];
    get_device_id(device_id, sizeof(device_id));
    get_mac_address(mac_str, sizeof(mac_str));
    get_ip_address(ip_str, sizeof(ip_str));
    get_device_name(device_name, sizeof(device_name));

    cJSON_AddStringToObject(state, "device_id", device_id);
    cJSON_AddStringToObject(state, "mac_address", mac_str);
    cJSON_AddStringToObject(state, "ip_address", ip_str);
    cJSON_AddStringToObject(state, "device_name", device_name);

    // Generate a random UUID for event_id
    char event_id[37];
    generate_uuid_v4(event_id, sizeof(event_id));
    cJSON_AddStringToObject(state, "event_id", event_id);

    // Assuming the first radar (index 0) is the one we want to report on
    radarButton_t *radar = &radars[0]; // Use the appropriate index or loop for specific radar if needed

    // Add average distance and average movement
    cJSON_AddNumberToObject(state, "distance", radar->last_avg_distance);
    cJSON_AddNumberToObject(state, "movement", radar->last_avg_movement);

    return state;
}

void sendRadarState(void) {
    cJSON *state = createRadarStateJson();
    if (!state) return;

    char *state_str = cJSON_PrintUnformatted(state);
    if (!state_str) {
        ESP_LOGE(RADAR_TAG, "Failed to serialize radar state to JSON string.");
        cJSON_Delete(state);
        return;
    }

    if (mqtt_client) {
        int msg_id = esp_mqtt_client_publish(mqtt_client, "radar", state_str, 0, 1, 0);
        ESP_LOGI(RADAR_TAG, "Published to MQTT topic 'radar' with msg_id=%d", msg_id);
    } else {
        ESP_LOGE(RADAR_TAG, "MQTT client not initialized");
    }

    free(state_str);
    cJSON_Delete(state);
}

void check_radar(int radar_index, uint8_t avg_movement) {
    bool above_threshold = (avg_movement > MOVEMENT_THRESHOLD);
    radarButton_t *radar = &radars[radar_index];
    char room_name[50];
    get_room_name(room_name, sizeof(room_name));

    snprintf(radar->event_description, sizeof(radar->event_description),
             "No significant radar activity in %s.", room_name);

    bool previous_presence_state = radar->presence_state;

    if (above_threshold) {
        radar->above_threshold_count++;
        radar->below_threshold_count = 0;
        if (radar->above_threshold_count >= HOLD_TIME_ABOVE_THRESHOLD && !radar->presence_state) {
            ESP_LOGI(RADAR_TAG, "Movement detected in %s. Movement Value: %d", room_name, avg_movement);
            radar->presence_state = true;
            radar->movement_value = avg_movement;
            snprintf(radar->event_description, sizeof(radar->event_description),
                     "Sustained Movement Detected in %s. Movement Value: %d", room_name, avg_movement);
            sendRadarState();
        }
    } else {
        radar->below_threshold_count++;
        radar->above_threshold_count = 0;
        if (radar->below_threshold_count >= HOLD_TIME_BELOW_THRESHOLD && radar->presence_state) {
            ESP_LOGI(RADAR_TAG, "No movement detected in %s. Movement Value: %d", room_name, avg_movement);
            radar->presence_state = false;
            radar->movement_value = avg_movement;
            snprintf(radar->event_description, sizeof(radar->event_description),
                     "No Significant Movement Detected in %s. Movement Value: %d", room_name, avg_movement);
            sendRadarState();
        }
    }
}

void log_data(uint8_t *data, int len, int radar_index) {
    if (!data || radar_index >= NUM_OF_RADARS || radar_index < 0) {
        ESP_LOGE(RADAR_TAG, "Invalid input parameters to log_data");
        return;
    }

    radarButton_t *radar = &radars[radar_index];

    // Current timestamp in milliseconds
    uint32_t current_time_ms = esp_timer_get_time() / 1000; // Convert microseconds to milliseconds

    // Static buffers to maintain rolling averages
    static uint16_t distance_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static uint8_t movement_buffer[NUM_OF_RADARS][ROLLING_AVG_SIZE] = {{0}};
    static int buffer_index[NUM_OF_RADARS] = {0};

    for (int data_index = 0; data_index + DATA_CHUNK_SIZE <= len; data_index += DATA_CHUNK_SIZE) {
        // Scan the entire 10-byte chunk for 0xAA
        for (int i = 0; i < DATA_CHUNK_SIZE; i++) {
            if (data[data_index + i] == 0xAA) {
                // Ensure there are enough bytes after 0xAA to form a complete packet
                if (i + 6 < DATA_CHUNK_SIZE) { // Adjust based on your packet structure
                    // Extract distance from bytes (assuming little endian)
                    uint16_t distance_value = (data[data_index + i + 2] << 8) | data[data_index + i + 1];

                    // Current distance in mm
                    uint16_t current_distance = distance_value;

                    // Calculate movement as change in distance over time
                    uint8_t movement_value = 0;
                    if (radar->has_previous) {
                        int16_t delta_distance = current_distance - radar->previous_distance;
                        uint32_t delta_time = current_time_ms - radar->previous_time_ms;
                        if (delta_time > 0) {
                            // Movement in mm/s, abs(delta_distance) to get magnitude
                            movement_value = (abs(delta_distance) * 1000) / delta_time;
                        }
                    }

                    // Update previous distance and time
                    radar->previous_distance = current_distance;
                    radar->previous_time_ms = current_time_ms;
                    radar->has_previous = true;

                    // Update rolling buffers
                    distance_buffer[radar_index][buffer_index[radar_index]] = current_distance;
                    movement_buffer[radar_index][buffer_index[radar_index]] = movement_value;

                    // Calculate Rolling Averages
                    uint32_t sum_distance = 0, sum_movement = 0;
                    for (int j = 0; j < ROLLING_AVG_SIZE; j++) {
                        sum_distance += distance_buffer[radar_index][j];
                        sum_movement += movement_buffer[radar_index][j];
                    }

                    uint16_t avg_distance = sum_distance / ROLLING_AVG_SIZE;
                    uint8_t avg_movement = sum_movement / ROLLING_AVG_SIZE;

                    radar->last_avg_distance = avg_distance;
                    radar->last_avg_movement = avg_movement;

                    buffer_index[radar_index] = (buffer_index[radar_index] + 1) % ROLLING_AVG_SIZE;
                    check_radar(radar_index, avg_movement);
                } else {
                    ESP_LOGV(RADAR_TAG, "Incomplete packet detected starting at byte %d in chunk", i);
                    // Handle incomplete packet if necessary
                }
            }
        }
    }
}

static void radar_service(void *pvParameter) {
    uint8_t *data = malloc(BUF_SIZE);
    if (!data) {
        ESP_LOGE(RADAR_TAG, "Failed to allocate memory for radar service buffer");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        for (int i = 0; i < NUM_OF_RADARS; i++) {
            int uart_num = (i == 0) ? UART_NUM_1 : UART_NUM_2;
            int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);
            if (len > 0) {
                log_data(data, len, i);
            }
        }

        cJSON *message = checkServiceMessage("radar");
        if (message) {
            handle_radar_message(message);
            cJSON_Delete(message);
        }

        vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
    }

    free(data); // Unreachable, but included for completeness
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(RADAR_TAG, "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(RADAR_TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(RADAR_TAG, "MQTT_EVENT_ERROR");
            return ESP_FAIL;
        default:
            ESP_LOGD(RADAR_TAG, "Other MQTT event: %d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(RADAR_TAG, "MQTT event handler called");
    mqtt_event_handler_cb(event_data);
}

void radar_main(int uart_pin_tx_1, int uart_pin_rx_1, int uart_pin_tx_2, int uart_pin_rx_2) {
    ESP_LOGI(RADAR_TAG, "Initializing Radar Service UART1 TX: %d RX: %d", uart_pin_tx_1, uart_pin_rx_1);
    ESP_LOGI(RADAR_TAG, "Initializing Radar Service UART2 TX: %d RX: %d", uart_pin_tx_2, uart_pin_rx_2);

    // Initialize UART for both radars
    init_uart(UART_NUM_1, uart_pin_tx_1, uart_pin_rx_1);
    init_uart(UART_NUM_2, uart_pin_tx_2, uart_pin_rx_2);

    // Initialize radars array
    memset(radars, 0, sizeof(radars));
    for (int i = 0; i < NUM_OF_RADARS; i++) {
        radars[i].channel = i + 1; // Channels typically start at 1
        strncpy(radars[i].room_name, "Office", sizeof(radars[i].room_name) - 1);
        radars[i].room_name[sizeof(radars[i].room_name) - 1] = '\0';
        strncpy(radars[i].event_description, "No significant radar activity.",
                sizeof(radars[i].event_description) - 1);
        radars[i].event_description[sizeof(radars[i].event_description) - 1] = '\0';
        radars[i].has_previous = false; // Initialize the flag
        radars[i].previous_distance = 0;
        radars[i].previous_time_ms = 0;
        radars[i].above_threshold_count = 0;
        radars[i].below_threshold_count = 0;
    }

    // Restore radar settings
    if (restoreRadarSettings() != 0) {
        ESP_LOGW(RADAR_TAG, "Failed to restore radar settings, using defaults");
    }

    // Create radar service task
    if (xTaskCreate(radar_service, "radar_service", 8 * 1024, NULL, 10, NULL) != pdPASS) {
        ESP_LOGE(RADAR_TAG, "Failed to create radar service task");
        return;
    }

    // Initialize MQTT client
    const char *mqtt_uri = "mqtt://192.168.1.42:1883";
    ESP_LOGI(RADAR_TAG, "Connecting to MQTT broker at %s", mqtt_uri);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = mqtt_uri,
        .credentials.username = "andy",
        .credentials.authentication.password = "qscwdvpk",
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client) {
        ESP_LOGE(RADAR_TAG, "Failed to initialize MQTT client");
        return;
    }

    if (esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL) != ESP_OK) {
        ESP_LOGE(RADAR_TAG, "Failed to register MQTT event handler");
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return;
    }

    if (esp_mqtt_client_start(mqtt_client) != ESP_OK) {
        ESP_LOGE(RADAR_TAG, "Failed to start MQTT client");
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return;
    }
}

void handle_radar_message(cJSON *payload) {
    if (!payload) {
        ESP_LOGE(RADAR_TAG, "Null payload received");
        return;
    }

    cJSON *channel = cJSON_GetObjectItemCaseSensitive(payload, "channel");
    cJSON *alert = cJSON_GetObjectItemCaseSensitive(payload, "alert");
    cJSON *delay = cJSON_GetObjectItemCaseSensitive(payload, "delay");

    if (cJSON_IsNumber(channel) && channel->valueint >= 1 && channel->valueint <= NUM_OF_RADARS) {
        int index = channel->valueint - 1;
        if (cJSON_IsBool(alert)) radars[index].alert = cJSON_IsTrue(alert);
        if (cJSON_IsNumber(delay)) radars[index].delay = delay->valueint;

        if (storeRadarSettings() != 0) {
            ESP_LOGE(RADAR_TAG, "Failed to store radar settings");
        }
        sendRadarState();
    } else {
        ESP_LOGE(RADAR_TAG, "Invalid channel value in radar message");
    }
}

void alertOnRadar(int ch, bool val) {
    if (ch >= 1 && ch <= NUM_OF_RADARS) {
        int index = ch - 1;
        radars[index].alert = val;
        sendRadarState();
    } else {
        ESP_LOGE(RADAR_TAG, "Invalid radar channel: %d", ch);
    }
}
