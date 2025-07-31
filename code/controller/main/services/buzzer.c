struct buzzer
{
	uint8_t pin;
	uint8_t beepCount;
	uint8_t longBeepCount;
	bool enable;
	bool contactAlert;
};

struct buzzer bzr;

void beep_keypad(int beeps, int duration) {
    if (!bzr.enable) return;

    for (int i = 0; i < beeps; i++) {
        gpio_set_level(bzr.pin, 1);
        vTaskDelay(duration / portTICK_PERIOD_MS);
        gpio_set_level(bzr.pin, 0);
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

    xTaskCreate(buzzer_task, "buzzer_task", 3072, NULL, 10, NULL);
}
