// hvac.c
#define HDC1080_ADDR 0x40 // HDC1080 default I2C address
#define HDC1080_TEMP_REG 0x00 // Temperature register
#define HDC1080_HUMIDITY_REG 0x01 // Humidity register

#define COOLING_STAGE_1_IO     Y1_IO // Define IO pins for Y1, Y2, W1, W2
#define COOLING_STAGE_2_IO     Y2_IO
#define HEATING_STAGE_1_IO     W1_IO
#define HEATING_STAGE_2_IO     W2_IO

const float TEMP_DIFFERENTIAL = 0.5; // Temperature differential to prevent oscillation
const float calibrationOffset = -7;

typedef struct {
    float setpointCool;
    float setpointHeat;
    float currentTemp;
    float currentHumidity;
    bool coolingEnabled;
    bool heatingEnabled;
} HVAC;

HVAC hvac;

void read_temperature() {
    uint8_t temp_reg = HDC1080_TEMP_REG;
    i2c_master_write_slave(HDC1080_ADDR, I2C_MASTER_NUM, &temp_reg, 1); // Set Pointer Register

    vTaskDelay(500 / portTICK_PERIOD_MS); // Increased wait time for conversion to reduce self-heating

    uint8_t temp_data[2];
    i2c_master_read_slave(HDC1080_ADDR, I2C_MASTER_NUM, temp_data, 2); // Read temperature data

    uint16_t raw_temp = (temp_data[0] << 8) | temp_data[1];
    hvac.currentTemp = (raw_temp / 65536.0) * 165.0 - 40.0; // Celsius

    // Calibration (if needed)
    hvac.currentTemp += calibrationOffset;

    // Convert to Fahrenheit
    float tempFahrenheit = (hvac.currentTemp * 9.0 / 5.0) + 32.0;

    // ESP_LOGI(TAG, "Raw Temp Data: %02x %02x", temp_data[0], temp_data[1]);
    // ESP_LOGI(TAG, "Temperature: %.2f°C / %.2f°F", hvac.currentTemp, tempFahrenheit);
}

// New Function to Read Humidity
void read_humidity() {
    uint8_t humidity_reg = HDC1080_HUMIDITY_REG;
    i2c_master_write_slave(HDC1080_ADDR, I2C_MASTER_NUM, &humidity_reg, 1); // Set Pointer Register to Humidity

    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for conversion

    uint8_t humidity_data[2];
    i2c_master_read_slave(HDC1080_ADDR, I2C_MASTER_NUM, humidity_data, 2); // Read humidity data

    uint16_t raw_humidity = (humidity_data[0] << 8) | humidity_data[1];
    hvac.currentHumidity = (raw_humidity / 65536.0) * 100.0; // Percentage

    // ESP_LOGI(TAG, "Raw Humidity Data: %02x %02x", humidity_data[0], humidity_data[1]);
    // ESP_LOGI(TAG, "Humidity: %.2f%%", hvac.currentHumidity);
}

void control_hvac() {
    // Cooling control
    if (hvac.coolingEnabled) {
        if (hvac.currentTemp > hvac.setpointCool + TEMP_DIFFERENTIAL) {
            set_io(COOLING_STAGE_1_IO, true); // Activate first stage of cooling
            if (hvac.currentTemp > hvac.setpointCool + 2 * TEMP_DIFFERENTIAL) {
                set_io(COOLING_STAGE_2_IO, true); // Activate second stage
            } else {
                set_io(COOLING_STAGE_2_IO, false);
            }
        } else {
            set_io(COOLING_STAGE_1_IO, false);
            set_io(COOLING_STAGE_2_IO, false);
        }
    }

    // Heating control
    if (hvac.heatingEnabled) {
        if (hvac.currentTemp < hvac.setpointHeat - TEMP_DIFFERENTIAL) {
            set_io(HEATING_STAGE_1_IO, true); // Activate first stage of heating
            if (hvac.currentTemp < hvac.setpointHeat - 2 * TEMP_DIFFERENTIAL) {
                set_io(HEATING_STAGE_2_IO, true); // Activate second stage
            } else {
                set_io(HEATING_STAGE_2_IO, false);
            }
        } else {
            set_io(HEATING_STAGE_1_IO, false);
            set_io(HEATING_STAGE_2_IO, false);
        }
    }
}

void hvac_init() {
    // Set default setpoints
    hvac.setpointCool = 24.0; // Default cooling setpoint
    hvac.setpointHeat = 20.0; // Default heating setpoint
    hvac.coolingEnabled = true;
    hvac.heatingEnabled = true;

    // Configure the sensor for 14-bit temperature resolution
    uint8_t config_reg = 0x02; // Configuration register address
    uint8_t config_value[2] = {0x00, 0x00}; // Default settings including 14-bit resolution
    i2c_master_write_slave(HDC1080_ADDR, I2C_MASTER_NUM, &config_reg, 1); // Point to config reg
    i2c_master_write_slave(HDC1080_ADDR, I2C_MASTER_NUM, config_value, 2); // Write config value
}

void hvac_main() {
    hvac_init();

    while (1) {
        read_temperature();
        read_humidity(); // New call to read humidity
        control_hvac();

        ESP_LOGI(TAG, "Temperature: %.2f°C / %.2f°F, Humidity: %.2f%%", 
            hvac.currentTemp, (hvac.currentTemp * 9.0 / 5.0) + 32.0, hvac.currentHumidity);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}