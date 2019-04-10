#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include "../lib/libwebsockets.h"
#endif

#include <string.h>

#define WS_PERIOD_US 5000

char utility_data_in[2000];
char utility_data_out[2000];
bool utility_data_out_ready = false;

struct pss__utility {
	int number;
};

struct vhd__utility {
	const unsigned int *options;
};

int
handle_utility_event()
{

	if (cJSON_GetObjectItem(utility_payload,"uuid")) {
		snprintf(device_id,sizeof(device_id),"%s",cJSON_GetObjectItem(utility_payload,"uuid")->valuestring);
		store_char("device_id",device_id);
		return -1;
	}
	return 0;
}

int
utility_event_handler(struct lws *wsi, cJSON * root)
{

	char uuid[100];
	if (cJSON_GetObjectItem(root,"payload")) {
		utility_payload = cJSON_GetObjectItemCaseSensitive(root,"payload");

		if (cJSON_GetObjectItemCaseSensitive(root,"id")) {
			int callback_id = cJSON_GetObjectItemCaseSensitive(root,"id")->valueint;
			char callback[70];
			snprintf(callback,sizeof(callback),"{\"id\":%d,\"callback\":true,\"payload\":[false,\"\"]}",callback_id);
			strcpy(utility_data_out,callback);
			utility_data_out_ready = true;
		}
	}
	return handle_utility_event();
}

static int
callback_utility(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	struct pss__utility *pss = (struct pss__utility *)user;
	struct vhd__utility *vhd =
				(struct vhd__utility *)
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
			sizeof(struct vhd__utility));
		if (!vhd)
			return -1;
		if ((opt = lws_pvo_search(
				(const struct lws_protocol_vhost_options *)in,
				"options")))
			vhd->options = (unsigned int *)opt->value;
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		//printf("LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER\n");
		utility_server_status = CONNECTED;
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lws_callback_on_writable(wsi);
		//set_pixel_by_index(0, 0, 255, 0, 1);
		setLED(0,255,0);
		pss->number = 0;
		strcpy(utility_data_in,"");
		if (!vhd->options || !((*vhd->options) & 1))
			lws_set_timer_usecs(wsi, WS_PERIOD_US);
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		//printf("LWS_CALLBACK_CLIENT_WRITEABLE\n");
		LWS_CALLBACK_CLIENT_WRITEABLE_CNT++;
		if (!utility_data_out_ready) break;
		n = lws_snprintf((char *)p, sizeof(utility_data_out) - LWS_PRE, "%s", utility_data_out);
		m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
		if (m < n) {
			lwsl_err("ERROR %d writing to token socket\n", n);
		} else {
			utility_data_out_ready = false;
			printf("SENT: %s\n",utility_data_out);
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		if (len < 10) break;
		if (len > 1000) break;
		//lwsl_notice("\n\nLWS_CALLBACK_RECEIVE(%d): %s\n\n",len,(const char *)in);
		//strcpy(utility_data_in,"");
		memset(&utility_data_in, 0, sizeof utility_data_in);
		strcpy(utility_data_in,(const char *)in);
		lwsl_notice("\n\nLWS_CALLBACK_RECEIVE %s\n\n",utility_data_in);
		//break;
		int valid_json = check_json(utility_data_in);
		cJSON *root = cJSON_Parse(utility_data_in);

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
		int res = utility_event_handler(wsi,root);
		if (res == 0) lwsl_notice("event_type not found\n");
		if (res == -1) {
			utility_server_status = DISCONNECTED;
			return -1;
		}
		break;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		lwsl_notice("\n\LWS_CALLBACK_WS_PEER_INITIATED_CLOSE (%d): %s\n\n",len,(const char *)in);
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		if (utility_server_status == CONNECTED) {
			utility_server_status = DISCONNECTED;
			return -1;
		}
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_notice("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY,
			(unsigned char *)"LWS_CALLBACK_CLIENT_CONNECTION_ERROR reconnecting...", 5);
		utility_server_status = DISCONNECTED;
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
		   	printf("utility-protocol callback: %d\n",reason);
			break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_UTILITY \
	{ \
		"utility-protocol", \
		callback_utility, \
		sizeof(struct pss__utility), \
		5000, /* rx buf size must be >= permessage-deflate rx size */ \
		0, NULL, 0 \
	}

#if !defined (LWS_PLUGIN_STATIC)

static const struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_UTILITY
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
