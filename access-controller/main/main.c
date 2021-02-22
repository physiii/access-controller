#define SERVICE_LOOP 20
#define SERVICE_LOOP_SHORT 10

#include "cJSON.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "automation.h"
#include "services/gpio.c"
#include "services/drivers/i2c.c"
#include "services/drivers/mcp23x17.c"
#include "services/store.c"
#include "services/buzzer.c"
#include "services/lock.c"
#include "services/exit.c"
#include "services/fob.c"
#include "services/server.c"
#include "services/ap.c"

void app_main(void)
{

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	serviceMessage.read = false;
	serviceMessage.message = NULL;

	gpio_main();
	i2c_main();
	mcp23x17_main();
	lock_main();
	buzzer_main();
	exit_main();
	fob_main();
	server_main();
	ap_main();

  ESP_ERROR_CHECK(example_connect());

	// turn_buzzer_on(false);
	int cnt = 0;
	while(1) {
		// printf("Disarming locks. \n");
		// arm_lock(1, false, ALERT);
		// arm_lock(2, false, ALERT);

		// printf("count %d\n", cnt++);
		vTaskDelay(5 * 1000 / portTICK_RATE_MS);
		// printf("Arming locks. \n");
		// arm_lock(1, true, ALERT);
		// arm_lock(2, true, ALERT);

		printf("count %d\n", cnt++);
		vTaskDelay(55 * 1000 / portTICK_RATE_MS);
		// printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
	}
}
