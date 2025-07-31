#define MOTION_MCP_IO_1 A0  // Changed from A6 to A0 for testing
#define MOTION_MCP_IO_2 B0  // Changed from B6 to B0 for testing
#define NUM_OF_MOTIONS				  2

#include "esp_timer.h"

char motion_service_message[2000];
bool motion_service_message_ready = false;
cJSON * motion_payload = NULL;

struct motionButton
{
  int pin;
	bool alert;
	bool isPressed;
	bool prevPress;
	int count;
	bool expired;
	bool enable;
	int delay;
	int channel;
	char settings[1000];
	char key[50];
	char type[40];
	cJSON *payload;
};

struct motionButton motions[NUM_OF_MOTIONS];

int storeMotionSettings()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char type[25] = "";
		strcpy(type, motions[i].type);
		sprintf(motions[i].settings,
			"{\"eventType\":\"%s\", "
			"\"payload\":{\"channel\":%d, \"enable\": %s, \"alert\": %s, \"delay\": %d}}",
			type,
			i+1,
			(motions[i].enable) ? "true" : "false",
			(motions[i].alert) ? "true" : "false",
			motions[i].delay);

		sprintf(motions[i].key, "%s%d", type, i);
		storeSetting(motions[i].key, cJSON_Parse(motions[i].settings));
		// printf("storeMotionSettings\t%s\n", motions[i].settings);
	}
  return 0;
}

int restoreMotionSettings()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char type[25] = "";
		strcpy(type, motions[i].type);
		sprintf(motions[i].key, "%s%d", type, i);
		restoreSetting(motions[i].key);
    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	}
	return 0;
}

void
sendMotionEventToServer()
{
	for (uint8_t i=0; i < NUM_OF_MOTIONS; i++) {
		char state_str[300];
		char msg[600];

		snprintf(state_str, sizeof(state_str), "{\"presence\":%s, \"exit\":false, \"keypad\":false, \"uptime\":1}", motions[i].isPressed ? "true" : "false");
		snprintf(msg, sizeof(msg),"{\"event_type\":\"load\", \"payload\":{\"services\":"
			"[{\"id\":\"ac_1\", \"type\":\"access-control\",\"state\":%s}]}}", state_str);

		addServerMessageToQueue(msg);
		printf("sendMotionEventToServer: %s\n", msg);
	}
}

void sendMotionEventToClient(int channel, bool state) {
    for (int i=0; i<NUM_OF_MOTIONS; i++) {
        if (motions[i].channel == channel) {
            if (strlen(motions[i].settings) > 2) {
                cJSON *json_msg = cJSON_Parse(motions[i].settings);
                addClientMessageToQueue(json_msg);
                cJSON_Delete(json_msg);
            }
        }
    }
}

void enableMotion (int ch, bool val)
{
	for (int i=0; i < NUM_OF_MOTIONS; i++)
		if (motions[i].channel == ch) motions[i].enable = val;
}

void alertOnMotion (int ch, bool val)
{
	for (int i=0; i < NUM_OF_MOTIONS; i++)
		if (motions[i].channel == ch) motions[i].alert = val;
}

void start_motion_timer (struct motionButton *mot, bool val)
{
  if (val) {
    mot->expired = false;
    mot->count = 0;
  } else {
    mot->expired = true;
  }
}

void check_motion (struct motionButton *mot)
{
	// Log each sensor individually with separate timers
	static int64_t last_debug_time[2] = {0, 0};
	int64_t current_time = esp_timer_get_time() / 1000;
	int sensor_index = mot->channel - 1; // Convert channel 1,2 to index 0,1
	
	if (sensor_index >= 0 && sensor_index < 2 && (current_time - last_debug_time[sensor_index]) > 3000) {
		ESP_LOGI(TAG, "=== Motion %d Debug ===", mot->channel);
		ESP_LOGI(TAG, "Motion %d: enable=%d, pin=%d", mot->channel, mot->enable, mot->pin);
		
		// Log actual MCP GPIO values
		bool raw_mcp_value = get_mcp_io(mot->pin);
		ESP_LOGI(TAG, "Motion %d: MCP raw_value=%d", mot->channel, raw_mcp_value);
		
		last_debug_time[sensor_index] = current_time;
	}

	if (!mot->enable) {
		ESP_LOGI(TAG, "Motion %d is DISABLED", mot->channel);
		return;
	}

	bool raw_value = get_mcp_io(mot->pin);
	mot->isPressed = !raw_value;
	
	// Always log state changes immediately
	if (mot->isPressed != mot->prevPress) {
		ESP_LOGI(TAG, "*** Motion %d STATE CHANGE: raw=%d, isPressed=%d, prevPress=%d ***", 
		         mot->channel, raw_value, mot->isPressed, mot->prevPress);
	}

	if (mot->isPressed && !mot->prevPress) {
		ESP_LOGI(TAG, "Motion detected on channel %d", mot->channel);
		arm_lock(mot->channel, false, mot->alert);
		start_motion_timer(mot, true);
	}

	mot->prevPress = mot->isPressed;
}

void handle_motion_message(cJSON * payload)
{
	int ch=0;
	bool tmp = 0;
	char state[250];

	if (payload == NULL) return;

	if (cJSON_GetObjectItem(payload,"getState")) {
		sendMotionEventToClient(motions[0].channel, motions[0].isPressed);
		sendMotionEventToServer();
	}

	if (cJSON_GetObjectItem(payload,"channel")) {
		 ch = cJSON_GetObjectItem(payload,"channel")->valueint;

		 if (cJSON_GetObjectItem(payload,"alert")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"alert"));
	 		alertOnMotion(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"enable")) {
	 		tmp = cJSON_IsTrue(cJSON_GetObjectItem(payload,"enable"));
	 		enableMotion(ch, tmp);
	 	}

	 	if (cJSON_GetObjectItem(payload,"delay")) {
	 		setArmDelay(ch, cJSON_GetObjectItem(payload,"delay")->valueint);
	 	}
		storeMotionSettings();
	}
}

void motion_timer_func(struct motionButton *mot)
{
	if (mot->count >= mot->delay && !mot->expired) {
		ESP_LOGI(TAG, "Re-arming lock from motion %d service.", mot->channel);
		arm_lock(mot->channel, true, mot->alert);
		mot->expired = true;
	} else {
		mot->count++;
	}
}

static void
motion_timer (void *pvParameter)
{
  while (1) {
		for (int i=0; i < NUM_OF_MOTIONS; i++)
			motion_timer_func(&motions[i]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void
motion_service (void *pvParameter)
{
  while (1) {
		static int loop_count = 0;
		loop_count++;
		
		// Log every 100 loops to see if service is running
		if (loop_count % 100 == 0) {
			ESP_LOGI(TAG, "Motion service loop %d, NUM_OF_MOTIONS=%d", loop_count, NUM_OF_MOTIONS);
		}
		
		for (int i=0; i < NUM_OF_MOTIONS; i++) {
			// Log which sensor we're checking
			if (loop_count % 100 == 0) {
				ESP_LOGI(TAG, "Checking motion sensor %d: channel=%d, pin=%d, enable=%d", 
				         i, motions[i].channel, motions[i].pin, motions[i].enable);
			}
			check_motion(&motions[i]);
		}

		handle_motion_message(checkServiceMessageByType("motion"));
    vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
  }
}

void motion_main()
{
  ESP_LOGI(TAG, "Starting motion service.");

	motions[0].pin = MOTION_MCP_IO_1;
	motions[0].delay = 4;
	motions[0].channel = 1;
	motions[0].alert = false;
	motions[0].enable = true;
	strcpy(motions[0].type, "motion");

	motions[1].pin = MOTION_MCP_IO_2;
	motions[1].delay = 4;
	motions[1].channel = 2;
	motions[1].alert = false;
	motions[1].enable = true;
	strcpy(motions[1].type, "motion");

	restoreMotionSettings();

  xTaskCreate(motion_timer, "motion_timer", 4096, NULL, 10, NULL);
	xTaskCreate(motion_service, "motion_service", 5000, NULL, 10, NULL);
}
