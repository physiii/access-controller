#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "cJSON.h"
#include <string.h>
#include "automation.h"
#include "services/gpio.c"
#include "services/drivers/i2c.c"
#include "services/drivers/mcp23x17.c"
// #include "services/store.c"
// #include "services/buzzer.c"
// #include "services/authorize.c"
// #include "services/lock.c"
// #include "services/wiegand.c"
// #include "services/exit.c"
// #include "services/keypad.c"
// #include "services/fob.c"
// #include "services/server.c"
// #include "services/ap.c"
// #include "services/station.c"

// #include "services/websocket.c"

// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "esp_netif.h"
// #include "protocol_examples_common.h"

void app_main(void)
{
// 	ESP_ERROR_CHECK(esp_netif_init());
// 	ESP_ERROR_CHECK(esp_event_loop_create_default());

// 	esp_err_t ret = nvs_flash_init();
// 	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
// 		ESP_ERROR_CHECK(nvs_flash_erase());
// 		ret = nvs_flash_init();
// 	}
// 	ESP_ERROR_CHECK(ret);

// 	serviceMessage.read = true;
// 	serviceMessage.message = NULL;

// 	clientMessage.readyToSend = false;

// 	gpio_main();

// 	if (USE_MCP23017) {
// 		i2c_main();
// 		mcp23x17_main();
// 	}

// 	auth_main();
// 	lock_main();
// 	buzzer_main();
// 	wiegand_main();
// 	exit_main();
// 	server_main();
// 	// ap_main();

//   ESP_ERROR_CHECK(example_connect());

// 	xTaskCreate(serviceMessageTask, "serviceMessageTask", 5000, NULL, 10, NULL);
// 	xTaskCreate(clientMessageTask, "clientMessageTask", 5000, NULL, 10, NULL);

	int cnt = 0;
	while(1) {
		vTaskDelay(60 * 1000 / portMAX_DELAY);
		printf("count: %d\tFree heap: %ld bytes\n", cnt++, esp_get_free_heap_size());
	}
}
