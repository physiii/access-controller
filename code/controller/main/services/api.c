#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"

#include "automation.h"

static const char *API_TAG = "api_server";

extern cJSON *lock_state_snapshot(void);
extern cJSON *exit_state_snapshot(void);
extern cJSON *fob_state_snapshot(void);

extern void handle_lock_message(cJSON *payload);
extern void handle_exit_message(cJSON *payload);
extern void handle_fob_message(cJSON *payload);
extern void handle_authorize_message(cJSON *payload);

extern char device_id[100];

static cJSON *build_state_snapshot(void) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON *device = cJSON_CreateObject();
    if (device) {
        cJSON_AddStringToObject(device, "uuid", device_id);
        cJSON_AddItemToObject(root, "device", device);
    }

    cJSON *locks = lock_state_snapshot();
    if (!locks) {
        locks = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "locks", locks);

    cJSON *exits = exit_state_snapshot();
    if (!exits) {
        exits = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "exits", exits);

    cJSON *fobs = fob_state_snapshot();
    if (!fobs) {
        fobs = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(root, "fobs", fobs);

    return root;
}

static esp_err_t send_json_response(httpd_req_t *req, cJSON *json) {
    if (!json) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build JSON response");
    }

    char *resp_str = cJSON_PrintUnformatted(json);
    if (!resp_str) {
        cJSON_Delete(json);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to serialise JSON");
    }

    httpd_resp_set_type(req, "application/json");
    esp_err_t result = httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(json);
    free(resp_str);
    return result;
}

static esp_err_t api_state_get_handler(httpd_req_t *req) {
    return send_json_response(req, build_state_snapshot());
}

static esp_err_t handle_json_post(httpd_req_t *req, void (*handler)(cJSON *), cJSON *(*state_builder)(void)) {
    const size_t max_len = 2048;
    int total_len = req->content_len;
    if (total_len <= 0 || total_len > (int)max_len) {
        ESP_LOGE(API_TAG, "Invalid content length: %d", total_len);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
    }

    char *buf = malloc(total_len + 1);
    if (!buf) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    }

    int received = 0;
    while (received < total_len) {
        int ret = httpd_req_recv(req, buf + received, total_len - received);
        if (ret <= 0) {
            free(buf);
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read request body");
        }
        received += ret;
    }
    buf[total_len] = '\0';

    cJSON *payload = cJSON_Parse(buf);
    free(buf);

    if (!payload) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    handler(cJSON_Duplicate(payload, 1));
    cJSON_Delete(payload);

    cJSON *state = state_builder ? state_builder() : cJSON_CreateObject();
    if (!state) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build state");
    }
    return send_json_response(req, state);
}

static void lock_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_lock_message(payload);
    }
}

static void exit_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_exit_message(payload);
    }
}

static void fob_message_wrapper(cJSON *payload) {
    if (payload) {
        handle_fob_message(payload);
    }
}

static esp_err_t api_lock_post_handler(httpd_req_t *req) {
    return handle_json_post(req, lock_message_wrapper, lock_state_snapshot);
}

static esp_err_t api_exit_post_handler(httpd_req_t *req) {
    return handle_json_post(req, exit_message_wrapper, exit_state_snapshot);
}

static esp_err_t api_fob_post_handler(httpd_req_t *req) {
    return handle_json_post(req, fob_message_wrapper, fob_state_snapshot);
}

static esp_err_t api_wifi_post_handler(httpd_req_t *req) {
    return handle_json_post(req, handle_authorize_message, build_state_snapshot);
}

static esp_err_t api_server_post_handler(httpd_req_t *req) {
    return handle_json_post(req, handle_authorize_message, build_state_snapshot);
}

static esp_err_t api_favicon_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/x-icon");
    return httpd_resp_send(req, NULL, 0);
}

void register_api_routes(httpd_handle_t server) {
    httpd_uri_t state = {
        .uri = "/api/state",
        .method = HTTP_GET,
        .handler = api_state_get_handler,
    };
    httpd_register_uri_handler(server, &state);

    httpd_uri_t lock_post = {
        .uri = "/api/lock",
        .method = HTTP_POST,
        .handler = api_lock_post_handler,
    };
    httpd_register_uri_handler(server, &lock_post);

    httpd_uri_t exit_post = {
        .uri = "/api/exit",
        .method = HTTP_POST,
        .handler = api_exit_post_handler,
    };
    httpd_register_uri_handler(server, &exit_post);

    httpd_uri_t fob_post = {
        .uri = "/api/fob",
        .method = HTTP_POST,
        .handler = api_fob_post_handler,
    };
    httpd_register_uri_handler(server, &fob_post);

    httpd_uri_t wifi_post = {
        .uri = "/api/wifi",
        .method = HTTP_POST,
        .handler = api_wifi_post_handler,
    };
    httpd_register_uri_handler(server, &wifi_post);

    httpd_uri_t server_post = {
        .uri = "/api/server",
        .method = HTTP_POST,
        .handler = api_server_post_handler,
    };
    httpd_register_uri_handler(server, &server_post);

    httpd_uri_t favicon = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = api_favicon_handler,
    };
    httpd_register_uri_handler(server, &favicon);
}

