static portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

#define WG_DELAY					80
#define NUM_OF_WIEGANDS			    2
#define NUM_OF_KEYS			    	12
#define KEYCODE_LENGTH	    		6

typedef struct {
    uint32_t gpio_num;
    int wg_index;
    int gpio_val; // Added this field to store the GPIO value
} gpio_event_t;

struct wiegand
{
	uint8_t pin0;
	uint8_t pin1;
	uint8_t pin_push;
	char code[25];
	uint8_t bitCount;
	char name[100];
	int count;
	int keypressCount;
	bool expired;
	bool keypressExpired;
	bool enable;
	bool newKey;
	bool newCode;
	char incomingCode[50];
	char fingerCode[50];
	uint8_t incomingCodeCount;
	int keyCount;
	bool alert;
	int delay;
	int keypressTimeout;
	int channel;
};

struct wiegand wg[NUM_OF_WIEGANDS];

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR wiegand_isr_handler(void* arg) {
	gpio_event_t event;
	event.gpio_num = (uint32_t) arg;
	event.gpio_val = gpio_get_level(event.gpio_num);
	xQueueSendFromISR(gpio_evt_queue, &event, NULL);
}

void start_keypress_timer (struct wiegand *wg, bool val)
{
	if (val) {
		wg->keypressExpired = false;
		wg->keypressCount = 0;
	} else {
		wg->keypressExpired = true;
	}
}

void check_keypress_timer (struct wiegand *wg)
{
	if (wg->keypressCount >= wg->keypressTimeout && !wg->keypressExpired) {
		printf("Keypress timer expired for wg %d\n", wg->channel);
		memset(wg->code, 0, sizeof(wg->code));
		memset(wg->incomingCode, 0, sizeof(wg->incomingCode));
		wg->incomingCodeCount = 0;
		wg->keypressExpired = true;
		wg->keypressCount = 0;
		beep_keypad(2, wg->channel);
	} else wg->keypressCount++;
}

static void keypress_timer (void *pvParameter)
{
  while (1) {
	for (int i=0; i < NUM_OF_WIEGANDS; i++)
		check_keypress_timer(&wg[i]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void start_wiegand_timer (struct wiegand *wg, bool val)
{
  start_keypress_timer(wg, false);
  if (val) {
    wg->expired = false;
    wg->count = 0;
  } else {
    wg->expired = true;
	wg->incomingCodeCount = 0;
  }
}

void check_wiegand_timer (struct wiegand *wg)
{
  if (!wg->enable) return;
  if (wg->count >= wg->delay && !wg->expired) {
		ESP_LOGI(TAG, "Re-arming lock from wg %d service. Alert %d", wg->channel, wg->alert);

		arm_lock(wg->channel, true, wg->alert);
		wg->expired = true;
  } else wg->count++;
}

static void wiegand_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_WIEGANDS; i++)
			check_wiegand_timer(&wg[i]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void handleKeyCode(struct wiegand *wg) {
	uint8_t key[12] = {
        0b00000000, // 0
        0b11000000, // 1
        0b00110000, // 2
        0b11110000, // 3
        0b00001100, // 4
        0b11001100, // 5
        0b00111100, // 6
        0b11111100, // 7
        0b00000011, // 8
        0b11000011, // 9
        0b00110011, // *
        0b11110011  // #
    };
    uint8_t incomingByte = (uint8_t) strtol(wg->incomingCode, NULL, 2);
    int keyIndex = -1;
    start_keypress_timer(wg, true);

    for(int i = 0; i < NUM_OF_KEYS; i++) {
        if (incomingByte == key[i]) {
            keyIndex = i;
            break;
        }
    }

    // Add the ASCII equivalent to the code string
    if (keyIndex >= 0 && keyIndex <= 9) {
        wg->code[strlen(wg->code)] = '0' + keyIndex;
    } else if (keyIndex == 10) {
        wg->code[strlen(wg->code)] = '*';
    } else if (keyIndex == 11) {
        // wg->code[strlen(wg->code)] = '#';

        if (is_pin_authorized(wg->code)) {
			arm_lock(wg->channel, false, wg->alert);
			start_wiegand_timer(wg, true);
		} else {
			beep_keypad(2, wg->channel);
		}
		memset(wg->code, 0, sizeof(wg->code));
		start_keypress_timer(wg, false);
        return;
    }

	if (strlen(wg->code) > KEYCODE_LENGTH) {
		printf("Exceeded max keycode length (%d): %s\n", KEYCODE_LENGTH, wg->code);
		memset(wg->code, 0, sizeof(wg->code));
		memset(wg->incomingCode, 0, sizeof(wg->incomingCode));
		wg->incomingCodeCount = 0;
		start_keypress_timer(wg, false);
		beep_keypad(2, wg->channel);
	}

    memset(wg->incomingCode, 0, sizeof(wg->incomingCode));
    wg->incomingCodeCount = 0;
}

static void wiegand_task(void *pvParameter) {
    gpio_event_t event;
    char codeStr[25];
    for (;;) {
        if(xQueueReceive(gpio_evt_queue, &event, portMAX_DELAY)) {
            struct wiegand *current_wg = &wg[event.wg_index];
            if (!current_wg->enable) continue;

            // Initialize incomingCode with zeros if count is zero
            if(current_wg->incomingCodeCount == 0) {
                memset(current_wg->incomingCode, '0', 8); // initialize all 8 bits to '0'
            }

            // Update the correct bit position
            int bitPosition = 8 - current_wg->incomingCodeCount - 1;
            if(event.gpio_num == current_wg->pin0 && event.gpio_val == 0) {
                current_wg->incomingCode[bitPosition] = '0';
            } else if(event.gpio_num == current_wg->pin1 && event.gpio_val == 0) {
                current_wg->incomingCode[bitPosition] = '1';
            }

            // Increment the bit count
            current_wg->incomingCodeCount++;

            // Log keypress and handle full 8-bit code
            if(current_wg->incomingCodeCount >= 8) {
                current_wg->incomingCode[8] = '\0'; // Null-terminate the 8-bit code
                printf("Keypress detected on pin of wg %d. Current code: %s\n", current_wg->channel, current_wg->incomingCode);
                handleKeyCode(current_wg);

                // Reset for the next round
                current_wg->incomingCodeCount = 0;
            }
        }
    }
}

void enableWiegand (int ch, bool val)
{
	for (int i=0; i < NUM_OF_WIEGANDS; i++)
		if (wg[i].channel == ch) wg[i].enable = val;
}

void wiegand_main() {
	wg[0].pin0 = WG0_DATA0_IO;
	wg[0].pin1 = WG0_DATA1_IO;
	wg[0].pin_push = OPEN_IO_1;
	wg[0].delay = 4;
	wg[0].keypressTimeout = 4;
	wg[0].channel = 1;
	wg[0].enable = true;
	wg[0].alert = true;
	wg[0].newKey = false;
	wg[0].incomingCode[0] = 0;
	strcpy(wg[0].name, "Wiegand0");

	wg[1].pin0 = WG1_DATA0_IO;
	wg[1].pin1 = WG1_DATA1_IO;
	wg[1].pin_push = OPEN_IO_1;
	wg[1].delay = 4;
	wg[1].keypressTimeout = 4;
	wg[1].channel = 2;
	wg[1].enable = false;
	wg[1].alert = true;
	wg[1].newKey = false;
	wg[1].incomingCode[0] = 0;
	strcpy(wg[1].name, "Wiegand1");

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(wiegand_timer, "wigand_timer", 3072, NULL, 10, NULL);
	xTaskCreate(keypress_timer, "keypress_timer", 3072, NULL, 10, NULL);
	xTaskCreate(wiegand_task, "wiegand_task", 4 * 1000, NULL, 10, NULL);

	for (int i=0; i < NUM_OF_WIEGANDS; i++) {
	  gpio_isr_handler_add(wg[i].pin0, wiegand_isr_handler, (void*) wg[i].pin0);
	  gpio_isr_handler_add(wg[i].pin1, wiegand_isr_handler, (void*) wg[i].pin1);
	}
}