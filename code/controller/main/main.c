static const char *TAG = "open-automation";

char device_id[100];
char token[700];

#include <esp_log.h>
#include "cJSON.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "automation.h"
#include "services/gpio.c"
#include "services/drivers/i2c.c"
#include "services/drivers/mcp23x17.c"
#include "services/store.c"
#include "services/authorize.c"
#include "services/buzzer.c"
#include "services/wiegand.c"
#include "services/lock.c"
#include "services/exit.c"
#include "services/radar.c"
#include "services/keypad.c"
#include "services/fob.c"
#include "services/server.c"
#include "services/ap.c"
#include "services/ws_client.c"

// void
// load_device()
// {
// 	char state_str[300];
// 	char msg[600];

// 	snprintf(state_str, sizeof(state_str), "{\"light_level\":51, \"time\":1, \"cycletime\":1, "
// 		"\"presence\":true, "
// 		"\"water_temp\":75, \"ec\":0.000, \"pco2\":4.1}");
// 	snprintf(msg, sizeof(msg),"{\"event_type\":\"load\", \"payload\":{\"services\":"
// 		"[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

// 	addServerMessageToQueue(msg);
// }

// void
// send_event()
// {
// 	char state_str[300];
// 	char msg[600];

// 	snprintf(state_str, sizeof(state_str), "{\"light_level\":51, \"time\":1, \"cycletime\":1, "
// 		"\"presence\":true, "
// 		"\"water_temp\":75, \"ec\":0.000, \"pco2\":4.1}");
// 	snprintf(msg, sizeof(msg),"{\"event_type\":\"event\", \"payload\":{\"services\":"
// 		"[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

// 	addServerMessageToQueue(msg);
// }

void app_main(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	strcpy(device_id, get_char("device_id"));
	if (strcmp(device_id, "")==0) {
		ESP_LOGI(TAG, "No Device ID found, fetching UUID...");
	  xTaskCreate(&ws_utilities_task, "ws_utilities_task", 10000, NULL, 5, NULL);
	} else {
		ESP_LOGI(TAG, "Device ID : %s", device_id);
	}

	strcpy(token, get_char("token"));
	if (strcmp(token,"")==0) {
		strcpy(token, device_id);
		ESP_LOGI(TAG, "No token found, setting as device id");
	} else {
		ESP_LOGI(TAG, "Token: %s", token);
	}

	serviceMessage.read = true;
	serviceMessage.message = NULL;

	clientMessage.readyToSend = false;
	serverMessage.readyToSend = false;

	gpio_main();

	if (STRIKE) {
		i2c_main();
		mcp23x17_main();
	}

	auth_main();
	lock_main();
	buzzer_main();
	// // wiegand_main();
	radar_main();
	exit_main();
	keypad_main();
	if (!STRIKE) fob_main();
	server_main();
	// // ap_main();

  	ESP_ERROR_CHECK(example_connect());

	ws_client_main();

	xTaskCreate(serviceMessageTask, "serviceMessageTask", 5000, NULL, 10, NULL);
	xTaskCreate(clientMessageTask, "clientMessageTask", 5000, NULL, 10, NULL);
	xTaskCreate(serverMessageTask, "serverMessageTask", 5000, NULL, 10, NULL);

	int cnt = 0;
	while(1) {
		// load_device();
		// send_event();

		printf("count %d\n", cnt++);
		vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
		printf("Minimum free heap size: %ld bytes\n", esp_get_minimum_free_heap_size());
	}
}
