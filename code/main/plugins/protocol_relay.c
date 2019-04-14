#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include "../lib/libwebsockets.h"
#endif

#include <string.h>

#define WS_PERIOD_US 5000

int LWS_CALLBACK_CLIENT_WRITEABLE_CNT = 0;
int data_part_count = 0;
char wss_data_in[2000];
char wss_data_out[2000];
bool wss_data_out_ready = false;
int callback_id = 0;

struct pss__wss {
	int number;
};

struct vhd__wss {
	const unsigned int *options;
};

int
char_count(char * ch1, char * ch2, char* str) {
	int m;
	int charcount = 0;

	charcount = 0;
	for(m=0; str[m]; m++) {
	    if(str[m] == ch1) {
	        charcount ++;
	    }
			if(str[m] == ch2) {
					charcount --;
			}
	}
	return charcount;
}

int
check_json(char * str)
{
	int bra_cnt = char_count("{","}",str);
	if (bra_cnt!=0) return 0;
	return 1;
}

int
add_headers(void *in, size_t len)
{
	char **h = (char **)in;

	if (len < 100)
		return 1;

	*h += sprintf(*h, "x-device-id: %s\x0d\x0a",device_id);
	*h += sprintf(*h, "x-device-type: %s\x0d\x0a","generic");
	*h += sprintf(*h, "x-device-token: %s\x0d\x0a",token);

	// printf("header token:\n%s\n",token);
	return 0;
}

int
handle_event(char * event_type)
{
	// printf("looking for event type: %s\n",event_type);
	if (strcmp(event_type,"dimmer")==0) {
		dimmer_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"alarm")==0) {
		alarm_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"motion")==0) {
		motion_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"button")==0) {
		button_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"microphone")==0) {
		microphone_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"schedule")==0) {
		schedule_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"lock")==0) {
		lock_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"nfc")==0) {
		nfc_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"log/get")==0) {
		log_payload = payload;
		payload = NULL;
		return 1;
	}

	if (strcmp(event_type,"load")==0) {
		char result[500];
		snprintf(result,sizeof(result),"%s",cJSON_GetObjectItem(payload,"result")->valuestring);
		lwsl_notice("loaded: %s\n", result);
		return 1;
	}

	if (strcmp(event_type,"token")==0) {
		snprintf(token,sizeof(token),"%s",cJSON_GetObjectItem(payload,"token")->valuestring);
		lwsl_notice("token received: %s\n", token);
		store_char("token",token);
		return 1;
	}

	if (strcmp(event_type,"reconnect-to-relay")==0) {
		lwsl_notice("reconnecting to relay!\n");
		return -1;
	}

	if (strcmp(event_type,"authentication")==0) {
		char error[500];
		snprintf(error,sizeof(error),"%s",cJSON_GetObjectItem(payload,"error")->valuestring);
		lwsl_err("websocket: %s\n", error);
		return 1;
	}

	if (strcmp(event_type,"time")==0) {
		current_time = cJSON_GetObjectItem(payload,"time")->valueint;
		schedule_payload = payload;
		payload = NULL;
		lwsl_notice("time: %d\n", current_time);
		return 1;
	}

	return 0;
}

int
wss_event_handler(struct lws *wsi, cJSON * root)
{

	if (cJSON_GetObjectItem(root,"event_type")) {
		char event_type[500];
		snprintf(event_type,sizeof(event_type),"%s",cJSON_GetObjectItem(root,"event_type")->valuestring);
		payload = cJSON_GetObjectItemCaseSensitive(root,"payload");

		// Get callback ID
		if (cJSON_GetObjectItemCaseSensitive(root,"id")) {
			callback_id = cJSON_GetObjectItemCaseSensitive(root,"id")->valueint;
		}

		return handle_event(event_type);
	}

	return 0;
}

static int
callback_wss(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	// return 0;
	struct pss__wss *pss = (struct pss__wss *)user;
	struct vhd__wss *vhd =
				(struct vhd__wss *)
				lws_protocol_vh_priv_get(lws_get_vhost(wsi),
						lws_get_protocol(wsi));
	uint8_t buf[LWS_PRE + 2000], *p = &buf[LWS_PRE];
	const struct lws_protocol_vhost_options *opt;
	int n, m;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		//lwsl_notice("!! ------- wss LWS_CALLBACK_PROTOCOL_INIT ---- !!\n");
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
			lws_get_protocol(wsi),
			sizeof(struct vhd__wss));
		if (!vhd)
			return -1;
		if ((opt = lws_pvo_search(
				(const struct lws_protocol_vhost_options *)in,
				"options")))
			vhd->options = (unsigned int *)opt->value;
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		//printf("LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER\n");
		relay_status = CONNECTED;
		add_headers(in,len);
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lws_callback_on_writable(wsi);
		//set_pixel_by_index(0, 0, 255, 0, 1);
		setLED(0,255,0);
		pss->number = 0;
		strcpy(wss_data_in,"");
		if (!vhd->options || !((*vhd->options) & 1))
			lws_set_timer_usecs(wsi, WS_PERIOD_US);
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		//printf("LWS_CALLBACK_CLIENT_WRITEABLE\n");
		if (!wss_data_out_ready) break;
		n = lws_snprintf((char *)p, sizeof(wss_data_out) - LWS_PRE, "%s", wss_data_out);
		m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
		if (m < n) {
			lwsl_err("ERROR %d writing to token socket\n", n);
		} else {
			wss_data_out_ready = false;
			// printf("SENT: %s\n",wss_data_out);
			strcpy(wss_data_out,"");
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		if (len < 10) break;
		if (len > 1000) break;
		// lwsl_notice("\n\nLWS_CALLBACK_RECEIVE(%d): %s\n\n",len,(const char *)in);

		memset(&wss_data_in, 0, sizeof wss_data_in);
		strcpy(wss_data_in,(const char *)in);
		int valid_json = check_json(wss_data_in);
		cJSON *root = cJSON_Parse(wss_data_in);

    if (root == NULL)
    {
				break;
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        valid_json = 0;
    }

		if (!valid_json) {
			lwsl_err("invalid incoming json\n");
			break;
		}
		int res = wss_event_handler(wsi,root);
		if (res == 0) lwsl_notice("event_type not found\n");
		if (res == -1) {
			relay_status = DISCONNECTED;
			return -1;
		}
		break;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		lwsl_notice("\nLWS_CALLBACK_WS_PEER_INITIATED_CLOSE\n");
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		lwsl_notice("LWS_CALLBACK_CLIENT_CLOSED\n");
		if (relay_status == CONNECTED) {
			relay_status = DISCONNECTED;
			return -1;
		}
		break;

	case LWS_CALLBACK_VHOST_CERT_AGING:
		lwsl_notice("LWS_CALLBACK_VHOST_CERT_AGING\n");
		break;

	case LWS_CALLBACK_WSI_CREATE:
		lwsl_notice("LWS_CALLBACK_WSI_CREATE\n");
		break;

	case LWS_CALLBACK_WS_CLIENT_DROP_PROTOCOL:
		lwsl_notice("LWS_CALLBACK_WS_CLIENT_DROP_PROTOCOL\n");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_notice("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY,
			(unsigned char *)"LWS_CALLBACK_CLIENT_CONNECTION_ERROR reconnecting...", 5);
		relay_status = DISCONNECTED;
		return -1;
		break;

	case LWS_CALLBACK_TIMER:
		if (!vhd->options || !((*vhd->options) & 1)) {
			lws_callback_on_writable_all_protocol_vhost(
				lws_get_vhost(wsi), lws_get_protocol(wsi));
			lws_set_timer_usecs(wsi, WS_PERIOD_US);
		}
		break;

	default:
		   	printf("wss-protocol callback: %d\n",reason);
			break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_WSS \
	{ \
		"wss-protocol", \
		callback_wss, \
		sizeof(struct pss__wss), \
		5000, /* rx buf size must be >= permessage-deflate rx size */ \
		0, NULL, 0 \
	}

#if !defined (LWS_PLUGIN_STATIC)

static const struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_WSS
};

LWS_EXTERN LWS_VISIBLE int
init_protocol_wss(struct lws_context *context,
			     struct lws_plugin_capability *c)
{
	if (c->api_magic != LWS_PLUGIN_API_MAGIC) {
		lwsl_err("Plugin API %d, library API %d", LWS_PLUGIN_API_MAGIC,
			 c->api_magic);
		return 1;
	}

	c->protocols = protocols;
	c->count_protocols = ARRAY_SIZE(protocols);
	c->extensions = NULL;
	c->count_extensions = 0;

	return 0;
}

LWS_EXTERN LWS_VISIBLE int
destroy_protocol_wss(struct lws_context *context)
{
	return 0;
}

#endif
