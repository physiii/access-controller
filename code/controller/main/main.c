#include "automation.h"
#include "services/drivers/i2c.c"
#include "services/drivers/mcp23x17.c"
#include "services/gpio.c"
#include "services/store.c"
#include "services/authorize.c"
#include "services/buzzer.c"
#include "services/lock.c"
#include "services/wiegand.c"
#include "services/exit.c"
#include "services/radar.c"
#include "services/keypad.c"
#include "services/fob.c"
#include "services/server.c"
#include "services/ap.c"
#include "services/ws_client.c"
#include "services/utilities_server.c"

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
		// store_char("device_id", "69696909-a5e5-401c-8da7-184fe2d5c844");
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

  	ESP_ERROR_CHECK(example_connect());
	ws_client_main();
	
	gpio_main();
	i2c_main();
	mcp23x17_main();
	auth_main();
	lock_main();
	buzzer_main();
	wiegand_main();

	// exit_main();
	// keypad_main();
	#if STRIKE
		radar_main();
	#else
		// fob_main();
		// ap_main();
	#endif
	server_main();

    TaskHandle_t serviceMessageTaskHandle = NULL;
    TaskHandle_t clientMessageTaskHandle = NULL;
    TaskHandle_t serverMessageTaskHandle = NULL;

    xTaskCreate(serviceMessageTask, "serviceMessageTask", 5000, NULL, 10, &serviceMessageTaskHandle);
    xTaskCreate(clientMessageTask, "clientMessageTask", 5000, NULL, 10, &clientMessageTaskHandle);
    xTaskCreate(serverMessageTask, "serverMessageTask", 5000, NULL, 10, &serverMessageTaskHandle);

    int cnt = 0;

    while(1) {
        sendUsers();

        printf("count %d\n", cnt++);
        
        // Log the minimum free heap size
        printf("Minimum free heap size: %ld bytes\n", esp_get_minimum_free_heap_size());

        // Check the stack's high water mark
        // if (serviceMessageTaskHandle != NULL) {
        //     printf("ServiceMessageTask High Water Mark: %d\n", uxTaskGetStackHighWaterMark(serviceMessageTaskHandle));
        // }
        // if (clientMessageTaskHandle != NULL) {
        //     printf("ClientMessageTask High Water Mark: %d\n", uxTaskGetStackHighWaterMark(clientMessageTaskHandle));
        // }
        // if (serverMessageTaskHandle != NULL) {
        //     printf("ServerMessageTask High Water Mark: %d\n", uxTaskGetStackHighWaterMark(serverMessageTaskHandle));
        // }

        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }
}
