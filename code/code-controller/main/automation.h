#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "rom/ets_sys.h"
#include "rom/crc.h"
#include "espnow_example.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

cJSON *payload = NULL;
cJSON *utility_payload = NULL;
cJSON *dimmer_payload = NULL;
cJSON *button_payload = NULL;
cJSON *LED_payload = NULL;
cJSON *schedule_payload = NULL;
cJSON *alarm_payload = NULL;
cJSON *motion_payload = NULL;
cJSON *microphone_payload = NULL;
cJSON *nfc_payload = NULL;
cJSON *lock_payload = NULL;
cJSON *log_payload = NULL;

bool isLockArmed = false;
void arm_lock(int,bool);
int set_switch(int);
int set_brightness(int);
void debounce_pir();
bool isArmed();
void createAlarmServiceMessage();
static int ratelimit_connects(unsigned int *last, unsigned int secs);

int seconds_in_minute = 60;
int minutes_in_hour = 60;
int hours_in_day = 24;
int central_time_hours = -6;

bool ENABLE_MOTION = false;
bool dimmer_enabled =  true;
char server_address[20] = "dev.pyfi.org";
int port = 443;
bool use_ssl = true;

int DISCONNECTED = 0;
int CONNECTING   = 1;
int CONNECTED    = 2;
int utility_server_status = 0;
int relay_status = 0;

struct lws *wsi_token;
int wsi_connect = 1;
unsigned int rl_token = 0;
unsigned int rl_ping = 0;
unsigned int rl_device_id = 0;
char token[1000];
char device_id[100];
bool start_service_loop = false;
bool token_received = false;
bool reconnect_with_token = false;
static struct lws_client_connect_info relay;
static struct lws_client_connect_info utility_server;
struct lws_vhost *vh;
bool sent_load_event = false;
char load_message[500];
static struct lws_context_creation_info info;
struct lws_context *context;

bool transmit_uid = true;
int current_time = 0;
bool got_ip = false;

struct access_log {
	char log_id[25];
  char date[25];
  char key_id[25];
  char name[50];
  bool registered;
  bool granted;
};

uint8_t keypad_code[4];
int code_size = 4;

#include "services/station.c"
#include "services/ota.c"
#include "services/store.c"
#include "services/lightswitch.c"
// #include "services/authorize.c"
// #include "services/LED.c"
// #include "plugins/protocol_relay.c"
// #include "plugins/protocol_utility.c"
// #include "services/log.c"
// #include "services/button.c"
// #include "services/motion.c"
// #include "services/lock.c"
// #include "services/keypad.c"
// #include "services/nfc.c"
