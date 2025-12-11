struct buzzer
{
	uint8_t pin;
	uint8_t beepCount;
	uint8_t longBeepCount;
	bool enable;
	bool contactAlert;
};

struct buzzer bzr;

// MCP23017 pins for keypad push/beep signal (active low to trigger keypad buzzer)
// From schematic: PUSH0_IO = GPA4 (index 4), PUSH1_IO = GPB4 (index 12)
#define MCP_PUSH0_IO  4   // Port A, pin 4
#define MCP_PUSH1_IO  12  // Port B, pin 4 (8 + 4 = 12)

// External MCP23017 functions
extern void set_mcp_io(uint8_t io, bool val);
extern void set_mcp_io_dir(uint8_t io, bool dir);

static const char *BUZZER_TAG = "buzzer";

void beep_keypad(int beeps, int channel) {
    if (!bzr.enable) return;

    // Select the correct PUSH pin based on channel (1 or 2)
    uint8_t push_pin = (channel == 2) ? MCP_PUSH1_IO : MCP_PUSH0_IO;

    ESP_LOGI(BUZZER_TAG, "beep_keypad: beeps=%d, channel=%d, MCP_PIN=%d", beeps, channel, push_pin);

    for (int i = 0; i < beeps; i++) {
        // Beep the onboard buzzer
        gpio_set_level(bzr.pin, 1);
        
        // Also pulse the keypad PUSH signal via MCP23017 (active low for most keypads)
        if (USE_MCP23017) {
            ESP_LOGI(BUZZER_TAG, "MCP PUSH pin %d -> LOW (active)", push_pin);
            set_mcp_io(push_pin, 0);  // Pull low to signal keypad
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        gpio_set_level(bzr.pin, 0);
        
        if (USE_MCP23017) {
            ESP_LOGI(BUZZER_TAG, "MCP PUSH pin %d -> HIGH (idle)", push_pin);
            set_mcp_io(push_pin, 1);  // Release high
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void buzzer_task(void *pvParameter) {
  while (1) {
    // This task can be used for more complex patterns in the future
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void buzzer_main() {
	bzr.pin = BUZZER_IO;
	bzr.enable = true;
	bzr.contactAlert = true;
	bzr.longBeepCount = 0;
	bzr.beepCount = 0;

    gpio_set_direction(bzr.pin, GPIO_MODE_OUTPUT);
    gpio_set_level(bzr.pin, 0);

    // Initialize MCP PUSH pins as outputs (high = idle, low = beep)
    if (USE_MCP23017) {
        ESP_LOGI(BUZZER_TAG, "Initializing MCP PUSH pins: PUSH0=%d, PUSH1=%d as outputs", MCP_PUSH0_IO, MCP_PUSH1_IO);
        
        // PUSH0 for channel 1
        set_mcp_io_dir(MCP_PUSH0_IO, 0);  // 0 = output
        set_mcp_io(MCP_PUSH0_IO, 1);       // Default high (inactive)
        
        // PUSH1 for channel 2
        set_mcp_io_dir(MCP_PUSH1_IO, 0);  // 0 = output
        set_mcp_io(MCP_PUSH1_IO, 1);       // Default high (inactive)
        
        ESP_LOGI(BUZZER_TAG, "MCP PUSH pins initialized to HIGH (idle)");
    }

    xTaskCreate(buzzer_task, "buzzer_task", 3072, NULL, 10, NULL);
}
