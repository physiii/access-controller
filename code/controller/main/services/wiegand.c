#define WG_DELAY					80
#define NUM_OF_WIEGANDS			    2
#define NUM_OF_KEYS			    	12
#define KEYCODE_LENGTH	    		6

struct wiegand
{
	uint8_t pin0;
	uint8_t pin1;
	uint8_t pin_push;
	uint64_t code;
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

static void IRAM_ATTR wiegand_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;

		for (int i=0; i < NUM_OF_WIEGANDS; i++) {
			if (!wg[i].enable) continue;

			// if (wg[i].code == 0) wg[i].newKey = true;
			wg[i].newKey = true;
			wg[i].bitCount++;

			if (gpio_num == wg[i].pin0) {
				wg[i].code = (wg[i].code << 1) | 0;
			} else if (gpio_num == wg[i].pin1) {
				wg[i].code = (wg[i].code << 1) | 1;
			}
		}
}

void start_wiegand_timer (struct wiegand *wg, bool val)
{
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
  if (wg->count >= wg->delay && !wg->expired) {
		printf("Re-arming lock from wg %d service. Alert %d\n", wg->channel, wg->alert);

		arm_lock(wg[0].channel, true, wg[0].alert);
		arm_lock(wg[1].channel, true, wg[1].alert);
		wg->expired = true;
  } else wg->count++;
}

static void
wiegand_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_WIEGANDS; i++)
			check_wiegand_timer(&wg[i]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void start_keypress_timer (struct wiegand *wg, bool val)
{
  if (val) {
    wg->keypressExpired = false;
    wg->keypressCount = 0;
  } else {
    wg->keypressExpired = true;
		wg->incomingCodeCount = 0;
  }
}

void check_keypress_timer (struct wiegand *wg)
{
  if (wg->keypressCount >= wg->keypressTimeout && !wg->keypressExpired) {
		wg->keypressExpired = true;
		wg->incomingCodeCount = 0;
  } else wg->keypressCount++;
}

static void
keypress_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_WIEGANDS; i++)
			check_keypress_timer(&wg[i]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void handleCode(struct wiegand *wg, char *codeStr) {
    if (current_mode == ADD_UID) {
        // add_auth_uid(codeStr);
    } else {
		if (is_pin_authorized(wg->incomingCode)) {
			arm_lock(wg->channel, false, wg->alert);
			start_wiegand_timer(wg, true);
		}
    }
}

void handleKeyCode(struct wiegand *wg) {
    printf("KEYCODE: %s\n", wg->incomingCode);
    handleCode(wg, wg->incomingCode);
    memset(wg->incomingCode, 0, sizeof(wg->incomingCode));
    wg->incomingCodeCount = 0;
}

void handleFingerCode(struct wiegand *wg) {
    printf("FINGER: %s\n", wg->fingerCode);
    handleCode(wg, wg->fingerCode);
    wg->code = 0;
    wg->bitCount = 0;
}

static void wiegand_task(void *pvParameter) {
    uint64_t keys[NUM_OF_KEYS] = {0, 3, 12, 15, 48, 51, 60, 63, 192, 195, 204, 207};
    char codeStr[25];

    for (;;) {
        for (int i = 0; i < NUM_OF_WIEGANDS; i++) {
            if (!wg[i].enable || !wg[i].newKey) {
                vTaskDelay(WG_DELAY / portTICK_PERIOD_MS);
                continue;
            }
            
            wg[i].newKey = false;
            printf("code (%d): %lld\n", wg[i].bitCount, wg[i].code);
            vTaskDelay(WG_DELAY / portTICK_PERIOD_MS);

            if (wg[i].code < 256) {
                start_keypress_timer(&wg[i], true);

                if (wg[i].bitCount == 52) {
                    handleFingerCode(&wg[i]);
                    continue;
                }

                for (int j = 0; j < NUM_OF_KEYS; j++) {
                    if (keys[j] != wg[i].code) continue;

                    if (j == 11) { // 11 represents pound key
                        handleKeyCode(&wg[i]);
                    } else {
                        wg[i].incomingCode[wg[i].incomingCodeCount] = '0' + j;
                        wg[i].incomingCodeCount++;

                        if (wg[i].incomingCodeCount > KEYCODE_LENGTH) {
                            printf("Reached max keycode length.\n");
                            memset(wg[i].incomingCode, 0, sizeof(wg[i].incomingCode));
                            wg[i].incomingCodeCount = 0;
                        }
                    }
                }
            } else {
                sprintf(codeStr, "%lld", wg[i].code);
                // handleCode(&wg[i], codeStr);
            }
            wg[i].code = 0;
            wg[i].bitCount = 0;
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
	// wg[0].pin_push = OPEN_IO_1;
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
	// wg[1].pin_push = OPEN_IO_1;
	wg[1].delay = 4;
	wg[1].keypressTimeout = 4;
	wg[1].channel = 2;
	wg[1].enable = false;
	wg[1].alert = true;
	wg[1].newKey = false;
	wg[1].incomingCode[0] = 0;
	strcpy(wg[1].name, "Wiegand1");

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(wiegand_timer, "wigand_timer", 2048, NULL, 10, NULL);
	xTaskCreate(keypress_timer, "keypress_timer", 2048, NULL, 10, NULL);
	xTaskCreate(wiegand_task, "wiegand_task", 2048, NULL, 10, NULL);

	for (int i=0; i < NUM_OF_WIEGANDS; i++) {
	  gpio_isr_handler_add(wg[i].pin0, wiegand_isr_handler, (void*) wg[i].pin0);
	  gpio_isr_handler_add(wg[i].pin1, wiegand_isr_handler, (void*) wg[i].pin1);
	}
}