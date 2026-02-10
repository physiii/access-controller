struct buzzer
{
	uint8_t pin;
	uint8_t beepCount;
	uint8_t longBeepCount;
	bool enable;
	bool contactAlert;
};

struct buzzer bzr;

// MCP23017 pins for keypad push/beep signal.
// Hardware: MCP pin drives an NPN transistor base (via resistor). So:
// - MCP HIGH  => transistor ON  => keypad PUSH line pulled LOW (active at the connector)
// - MCP LOW   => transistor OFF => keypad PUSH line released (idle)
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
        
        // Also pulse the keypad PUSH signal via MCP23017.
        // NOTE: This pin drives a transistor, so we pulse HIGH to assert the PUSH line.
        if (USE_MCP23017) {
            ESP_LOGI(BUZZER_TAG, "MCP PUSH pin %d -> HIGH (active)", push_pin);
            set_mcp_io(push_pin, 1);  // Assert (turn transistor on)
        } else {
            // Direct GPIO fallback for boards that route PUSH via ESP32 GPIO.
            // Same rule: HIGH asserts (turns the transistor on), LOW releases.
            gpio_set_level(OPEN_IO_1, 1);
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        gpio_set_level(bzr.pin, 0);
        
        if (USE_MCP23017) {
            ESP_LOGI(BUZZER_TAG, "MCP PUSH pin %d -> LOW (idle)", push_pin);
            set_mcp_io(push_pin, 0);  // Deassert (turn transistor off)
        } else {
            gpio_set_level(OPEN_IO_1, 0);
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

    // Ensure keypad PUSH output is OFF by default (idle).
    // (OPEN_IO_1 is configured as an output in gpio_main().)
    gpio_set_level(OPEN_IO_1, 0);

    // Initialize MCP PUSH pins as outputs (LOW = idle, HIGH = pulse)
    if (USE_MCP23017) {
        ESP_LOGI(BUZZER_TAG, "Initializing MCP PUSH pins: PUSH0=%d, PUSH1=%d as outputs", MCP_PUSH0_IO, MCP_PUSH1_IO);
        
        // PUSH0 for channel 1
        set_mcp_io_dir(MCP_PUSH0_IO, 0);  // 0 = output
        set_mcp_io(MCP_PUSH0_IO, 0);       // Default low (idle)
        
        // PUSH1 for channel 2
        set_mcp_io_dir(MCP_PUSH1_IO, 0);  // 0 = output
        set_mcp_io(MCP_PUSH1_IO, 0);       // Default low (idle)
        
        ESP_LOGI(BUZZER_TAG, "MCP PUSH pins initialized to LOW (idle)");
    }

    xTaskCreate(buzzer_task, "buzzer_task", 3072, NULL, 10, NULL);
}
