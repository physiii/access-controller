#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"

#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "automation.h"

extern char *get_char(const char *key);
extern cJSON *lock_state_snapshot(void);
extern cJSON *exit_state_snapshot(void);
extern cJSON *fob_state_snapshot(void);

static const char *TUNNEL_TAG = "tunnel";

#define TUNNEL_DEFAULT_HOST "192.168.1.43"
#define TUNNEL_DEFAULT_PORT 9001
#define TUNNEL_RECONNECT_DELAY_MS 5000
#define TUNNEL_MAX_HEADER_BYTES (64 * 1024)
#define TUNNEL_MAX_BODY_BYTES   (128 * 1024)
#define LOCAL_HTTP_TIMEOUT_MS   8000

typedef struct {
    char host[64];
    int port;
} tunnel_config_t;

typedef struct {
    int sock;
    bool identified;
    char assigned_id[100];
    tunnel_config_t config;
} tunnel_client_t;

typedef struct {
    char key[64];
    char value[128];
} header_pair_t;

#define MAX_STREAM_HEADERS 20

typedef struct {
    tunnel_client_t *client;
    char request_id[64];
    header_pair_t headers[MAX_STREAM_HEADERS];
    int header_count;
    bool start_sent;
    bool finished;
    bool error;
    esp_err_t last_err;
} http_stream_context_t;

static void add_header_to_json(cJSON *headers_obj, const char *key, const char *value);
static void handle_abort_frame(tunnel_client_t *client, cJSON *header);

static const char *normalize_target(const char *target, char *buffer, size_t buffer_len) {
    if (!target) {
        return "/";
    }

    if (strncmp(target, "/device/", 8) == 0) {
        const char *after_id = strchr(target + 8, '/');
        if (after_id) {
            strlcpy(buffer, after_id, buffer_len);
            return buffer;
        }
        strlcpy(buffer, "/", buffer_len);
        return buffer;
    }

    return target;
}

static bool tunnel_task_started = false;

void tunnel_ws_broadcast(const char *message) {
    (void)message;
}

static void load_tunnel_config(tunnel_config_t *cfg) {
    if (!cfg) {
        return;
    }

    char *stored_host = get_char("tunnel_host");
    char *stored_port = get_char("tunnel_port");
    char *server_host = get_char("server_ip");
    char *server_port = get_char("server_port");

    const char *host_pick = (stored_host && stored_host[0]) ? stored_host :
                            (server_host && server_host[0]) ? server_host : TUNNEL_DEFAULT_HOST;
    const char *port_pick_str = (stored_port && stored_port[0]) ? stored_port :
                                (server_port && server_port[0]) ? server_port : NULL;

    snprintf(cfg->host, sizeof(cfg->host), "%s", host_pick);
    if (port_pick_str) {
        cfg->port = atoi(port_pick_str);
    } else {
        cfg->port = TUNNEL_DEFAULT_PORT;
    }

    if (cfg->port <= 0) {
        cfg->port = TUNNEL_DEFAULT_PORT;
    }

    if (stored_host) free(stored_host);
    if (stored_port) free(stored_port);
    if (server_host) free(server_host);
    if (server_port) free(server_port);
}

static esp_http_client_method_t http_method_from_string(const char *method) {
    if (!method) {
        return HTTP_METHOD_GET;
    }

    if (strcasecmp(method, "GET") == 0) return HTTP_METHOD_GET;
    if (strcasecmp(method, "POST") == 0) return HTTP_METHOD_POST;
    if (strcasecmp(method, "PUT") == 0) return HTTP_METHOD_PUT;
    if (strcasecmp(method, "DELETE") == 0) return HTTP_METHOD_DELETE;
    if (strcasecmp(method, "PATCH") == 0) return HTTP_METHOD_PATCH;
    if (strcasecmp(method, "HEAD") == 0) return HTTP_METHOD_HEAD;
    if (strcasecmp(method, "OPTIONS") == 0) return HTTP_METHOD_OPTIONS;
    return HTTP_METHOD_GET;
}

static esp_err_t send_all(int sock, const uint8_t *data, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        int sent = send(sock, data + total_sent, len - total_sent, 0);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            ESP_LOGE(TUNNEL_TAG, "send failed: errno=%d", errno);
            return ESP_FAIL;
        }
        if (sent == 0) {
            ESP_LOGW(TUNNEL_TAG, "send returned 0 bytes");
            return ESP_FAIL;
        }
        total_sent += (size_t)sent;
    }
    return ESP_OK;
}

static esp_err_t read_exact(int sock, uint8_t *buf, size_t len) {
    size_t total_read = 0;
    while (total_read < len) {
        int received = recv(sock, buf + total_read, len - total_read, 0);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            ESP_LOGE(TUNNEL_TAG, "recv failed: errno=%d", errno);
            return ESP_FAIL;
        }
        if (received == 0) {
            ESP_LOGW(TUNNEL_TAG, "Connection closed by peer");
            return ESP_FAIL;
        }
        total_read += (size_t)received;
    }
    return ESP_OK;
}

static esp_err_t send_frame(int sock, cJSON *header, const uint8_t *body, size_t body_len) {
    if (!header) {
        return ESP_ERR_INVALID_ARG;
    }

    cJSON_DeleteItemFromObjectCaseSensitive(header, "bodyLength");
    cJSON_AddNumberToObject(header, "bodyLength", (double)body_len);

    char *header_str = cJSON_PrintUnformatted(header);
    if (!header_str) {
        ESP_LOGE(TUNNEL_TAG, "Failed to serialize frame header");
        return ESP_FAIL;
    }

    size_t header_len = strlen(header_str);
    if (header_len > TUNNEL_MAX_HEADER_BYTES) {
        ESP_LOGE(TUNNEL_TAG, "Frame header too large (%u bytes)", (unsigned int)header_len);
        cJSON_free(header_str);
        return ESP_ERR_NO_MEM;
    }

    uint32_t header_len_be = htonl((uint32_t)header_len);
    esp_err_t err = send_all(sock, (uint8_t *)&header_len_be, sizeof(header_len_be));
    if (err != ESP_OK) {
        ESP_LOGE(TUNNEL_TAG, "Failed to send frame header length");
        cJSON_free(header_str);
        return err;
    }

    err = send_all(sock, (uint8_t *)header_str, header_len);
    if (err != ESP_OK) {
        ESP_LOGE(TUNNEL_TAG, "Failed to send frame header body");
        cJSON_free(header_str);
        return err;
    }

    cJSON_free(header_str);

    if (body_len > 0 && body) {
        err = send_all(sock, body, body_len);
        if (err != ESP_OK) {
            ESP_LOGE(TUNNEL_TAG, "Failed to send frame payload");
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t send_simple_frame(int sock, const char *type) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return ESP_ERR_NO_MEM;
    }
    cJSON_AddStringToObject(root, "type", type);
    esp_err_t err = send_frame(sock, root, NULL, 0);
    cJSON_Delete(root);
    return err;
}

static esp_err_t send_http_error(int sock, const char *request_id, const char *message) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return ESP_ERR_NO_MEM;
    }
    cJSON_AddStringToObject(root, "type", "httpError");
    if (request_id) {
        cJSON_AddStringToObject(root, "requestId", request_id);
    }
    if (message) {
        cJSON_AddStringToObject(root, "message", message);
    }
    esp_err_t err = send_frame(sock, root, NULL, 0);
    cJSON_Delete(root);
    return err;
}

static esp_err_t send_http_response_start(http_stream_context_t *ctx, esp_http_client_handle_t http_client) {
    if (!ctx || ctx->start_sent) {
        return ESP_OK;
    }

    cJSON *response_header = cJSON_CreateObject();
    if (!response_header) {
        return ESP_ERR_NO_MEM;
    }

    cJSON_AddStringToObject(response_header, "type", "httpResponseStart");
    cJSON_AddStringToObject(response_header, "requestId", ctx->request_id);

    int status_code = esp_http_client_get_status_code(http_client);
    cJSON_AddNumberToObject(response_header, "statusCode", status_code);

    cJSON *headers_obj = cJSON_CreateObject();
    if (!headers_obj) {
        cJSON_Delete(response_header);
        return ESP_ERR_NO_MEM;
    }

    for (int i = 0; i < ctx->header_count; ++i) {
        add_header_to_json(headers_obj, ctx->headers[i].key, ctx->headers[i].value);
    }

    int64_t content_length = esp_http_client_get_content_length(http_client);
    if (content_length >= 0 && !cJSON_HasObjectItem(headers_obj, "content-length")) {
        char content_length_str[24];
        snprintf(content_length_str, sizeof(content_length_str), "%lld", (long long)content_length);
        add_header_to_json(headers_obj, "content-length", content_length_str);
    }

    if (!cJSON_HasObjectItem(headers_obj, "connection")) {
        add_header_to_json(headers_obj, "connection", "close");
    }

    if (!cJSON_HasObjectItem(headers_obj, "content-type")) {
        char *content_type = NULL;
        if (esp_http_client_get_header(http_client, "Content-Type", &content_type) == ESP_OK && content_type) {
            add_header_to_json(headers_obj, "content-type", content_type);
        }
    }

    cJSON_AddItemToObject(response_header, "headers", headers_obj);

    esp_err_t err = send_frame(ctx->client->sock, response_header, NULL, 0);
    cJSON_Delete(response_header);
    if (err == ESP_OK) {
        ctx->start_sent = true;
    }
    return err;
}

static esp_err_t send_http_response_chunk(http_stream_context_t *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data || len == 0) {
        return ESP_OK;
    }

    cJSON *chunk_header = cJSON_CreateObject();
    if (!chunk_header) {
        return ESP_ERR_NO_MEM;
    }

    cJSON_AddStringToObject(chunk_header, "type", "httpResponseChunk");
    cJSON_AddStringToObject(chunk_header, "requestId", ctx->request_id);

    esp_err_t err = send_frame(ctx->client->sock, chunk_header, data, len);
    cJSON_Delete(chunk_header);
    return err;
}

static esp_err_t send_http_response_end(http_stream_context_t *ctx) {
    if (!ctx || ctx->finished) {
        return ESP_OK;
    }

    cJSON *end_header = cJSON_CreateObject();
    if (!end_header) {
        return ESP_ERR_NO_MEM;
    }

    cJSON_AddStringToObject(end_header, "type", "httpResponseEnd");
    cJSON_AddStringToObject(end_header, "requestId", ctx->request_id);

    esp_err_t err = send_frame(ctx->client->sock, end_header, NULL, 0);
    cJSON_Delete(end_header);
    if (err == ESP_OK) {
        ctx->finished = true;
    }
    return err;
}

static esp_err_t http_stream_event_handler(esp_http_client_event_t *evt) {
    http_stream_context_t *ctx = (http_stream_context_t *)evt->user_data;
    if (!ctx) {
        return ESP_OK;
    }

    switch (evt->event_id) {
    case HTTP_EVENT_ON_HEADER:
        if (evt->header_key && evt->header_value && ctx->header_count < MAX_STREAM_HEADERS) {
            snprintf(ctx->headers[ctx->header_count].key,
                     sizeof(ctx->headers[ctx->header_count].key),
                     "%s", evt->header_key);
            snprintf(ctx->headers[ctx->header_count].value,
                     sizeof(ctx->headers[ctx->header_count].value),
                     "%s", evt->header_value);
            ctx->header_count++;
        }
        break;
    case HTTP_EVENT_ON_DATA: {
        if (!ctx->start_sent) {
            esp_err_t err = send_http_response_start(ctx, evt->client);
            if (err != ESP_OK) {
                ctx->error = true;
                ctx->last_err = err;
                break;
            }
        }
        if (evt->data && evt->data_len > 0) {
            esp_err_t err = send_http_response_chunk(ctx, (const uint8_t *)evt->data, evt->data_len);
            if (err != ESP_OK) {
                ctx->error = true;
                ctx->last_err = err;
            }
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH:
        if (!ctx->start_sent) {
            esp_err_t err = send_http_response_start(ctx, evt->client);
            if (err != ESP_OK) {
                ctx->error = true;
                ctx->last_err = err;
            }
        }
        if (!ctx->finished) {
            esp_err_t err = send_http_response_end(ctx);
            if (err != ESP_OK) {
                ctx->error = true;
                ctx->last_err = err;
            }
        }
        break;
    case HTTP_EVENT_DISCONNECTED:
        if (ctx->start_sent && !ctx->finished) {
            esp_err_t err = send_http_response_end(ctx);
            if (err != ESP_OK) {
                ctx->last_err = err;
            }
        }
        ctx->finished = true;
        break;
    case HTTP_EVENT_ERROR:
        ctx->error = true;
        break;
    default:
        break;
    }

    return ESP_OK;
}

static void add_header_to_json(cJSON *headers_obj, const char *key, const char *value) {
    if (!headers_obj || !key || !value) {
        return;
    }

    char lower_key[64];
    size_t i;
    for (i = 0; i < sizeof(lower_key) - 1 && key[i]; ++i) {
        lower_key[i] = (char)tolower((unsigned char)key[i]);
    }
    lower_key[i] = '\0';

    cJSON *array = cJSON_GetObjectItemCaseSensitive(headers_obj, lower_key);
    if (!array) {
        array = cJSON_CreateArray();
        if (!array) {
            return;
        }
        cJSON_AddItemToObject(headers_obj, lower_key, array);
    }

    cJSON_AddItemToArray(array, cJSON_CreateString(value));
}

static void handle_abort_frame(tunnel_client_t *client, cJSON *header) {
    (void)client;
    if (!header) {
        return;
    }
    cJSON *request_id_item = cJSON_GetObjectItemCaseSensitive(header, "requestId");
    const char *request_id = cJSON_IsString(request_id_item) ? request_id_item->valuestring : NULL;
    if (request_id && request_id[0] != '\0') {
        ESP_LOGI(TUNNEL_TAG, "Request %s aborted by tunnel server", request_id);
    } else {
        ESP_LOGI(TUNNEL_TAG, "Tunnel server aborted request");
    }
}

static esp_err_t forward_request_to_local_http(tunnel_client_t *client, const char *request_id, const char *method,
                                               const char *target, cJSON *headers, const uint8_t *body, size_t body_len) {
    if (!client || !request_id || !method || !target) {
        return ESP_ERR_INVALID_ARG;
    }

    if (body_len > TUNNEL_MAX_BODY_BYTES) {
        ESP_LOGW(TUNNEL_TAG, "Request body too large (%u bytes)", (unsigned int)body_len);
        return send_http_error(client->sock, request_id, "Request body too large");
    }

    char normalized_path[256];
    const char *local_path = normalize_target(target, normalized_path, sizeof(normalized_path));

    char url[256];
    snprintf(url, sizeof(url), "http://127.0.0.1%s", (*local_path) ? local_path : "/");

    esp_http_client_config_t http_cfg = {
        .url = url,
        .method = http_method_from_string(method),
        .timeout_ms = LOCAL_HTTP_TIMEOUT_MS,
        .event_handler = http_stream_event_handler,
        .buffer_size = 1024,
        .buffer_size_tx = 1024,
    };

    http_stream_context_t stream_ctx = {
        .client = client,
        .header_count = 0,
        .start_sent = false,
        .finished = false,
        .error = false,
        .last_err = ESP_OK,
    };
    snprintf(stream_ctx.request_id, sizeof(stream_ctx.request_id), "%s", request_id);
    http_cfg.user_data = &stream_ctx;

    esp_http_client_handle_t http_client = esp_http_client_init(&http_cfg);
    if (!http_client) {
        return send_http_error(client->sock, request_id, "Failed to init local HTTP client");
    }

    if (headers && cJSON_IsObject(headers)) {
        cJSON *header = NULL;
        cJSON_ArrayForEach(header, headers) {
            const char *key = header->string;
            if (!key || !cJSON_IsArray(header)) {
                continue;
            }
            cJSON *value_item = cJSON_GetArrayItem(header, 0);
            if (!value_item || !cJSON_IsString(value_item)) {
                continue;
            }
            if (strcasecmp(key, "content-length") == 0) {
                continue;
            }
            if (strcasecmp(key, "connection") == 0) {
                continue;
            }
            esp_http_client_set_header(http_client, key, value_item->valuestring);
        }
    }

    esp_http_client_set_header(http_client, "Connection", "close");
    if (body && body_len > 0) {
        esp_http_client_set_post_field(http_client, (const char *)body, (int)body_len);
    }

    esp_err_t err = esp_http_client_perform(http_client);
    if (err != ESP_OK) {
        ESP_LOGE(TUNNEL_TAG, "Local HTTP request failed: %s", esp_err_to_name(err));
    }

    if (!stream_ctx.start_sent && !stream_ctx.error) {
        esp_err_t start_err = send_http_response_start(&stream_ctx, http_client);
        if (start_err != ESP_OK) {
            stream_ctx.error = true;
            stream_ctx.last_err = start_err;
        }
    }

    if (stream_ctx.start_sent && !stream_ctx.finished) {
        esp_err_t end_err = send_http_response_end(&stream_ctx);
        if (end_err != ESP_OK) {
            stream_ctx.error = true;
            stream_ctx.last_err = end_err;
        }
    }

    esp_http_client_cleanup(http_client);

    if (err != ESP_OK || stream_ctx.error) {
        if (!stream_ctx.start_sent) {
            return send_http_error(client->sock, request_id, "Local HTTP request failed");
        }
        if (!stream_ctx.finished) {
            send_http_response_end(&stream_ctx);
        }
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t handle_http_request_frame(tunnel_client_t *client, cJSON *header, const uint8_t *body, size_t body_len) {
    if (!client || !header) {
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *request_id_item = cJSON_GetObjectItemCaseSensitive(header, "requestId");
    cJSON *method_item = cJSON_GetObjectItemCaseSensitive(header, "method");
    cJSON *target_item = cJSON_GetObjectItemCaseSensitive(header, "target");
    cJSON *headers_item = cJSON_GetObjectItemCaseSensitive(header, "headers");

    if (!cJSON_IsString(request_id_item) || !cJSON_IsString(method_item) || !cJSON_IsString(target_item)) {
        ESP_LOGE(TUNNEL_TAG, "Invalid HTTP request frame");
        return send_http_error(client->sock, NULL, "Malformed httpRequest frame");
    }

    return forward_request_to_local_http(client,
                                         request_id_item->valuestring,
                                         method_item->valuestring,
                                         target_item->valuestring,
                                         headers_item,
                                         body,
                                         body_len);
}

static esp_err_t read_frame(int sock, cJSON **header_out, uint8_t **body_out, size_t *body_len_out) {
    if (!header_out || !body_out || !body_len_out) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t header_len_be = 0;
    esp_err_t err = read_exact(sock, (uint8_t *)&header_len_be, sizeof(header_len_be));
    if (err != ESP_OK) {
        return err;
    }

    uint32_t header_len = ntohl(header_len_be);
    if (header_len == 0 || header_len > TUNNEL_MAX_HEADER_BYTES) {
        ESP_LOGE(TUNNEL_TAG, "Invalid frame header length: %u", header_len);
        return ESP_FAIL;
    }

    char *header_buf = malloc(header_len + 1);
    if (!header_buf) {
        return ESP_ERR_NO_MEM;
    }

    err = read_exact(sock, (uint8_t *)header_buf, header_len);
    if (err != ESP_OK) {
        free(header_buf);
        return err;
    }
    header_buf[header_len] = '\0';

    cJSON *header = cJSON_Parse(header_buf);
    free(header_buf);
    if (!header) {
        ESP_LOGE(TUNNEL_TAG, "Failed to parse frame header JSON");
        return ESP_FAIL;
    }

    size_t body_len = 0;
    cJSON *body_len_item = cJSON_GetObjectItemCaseSensitive(header, "bodyLength");
    if (cJSON_IsNumber(body_len_item)) {
        double len_value = body_len_item->valuedouble;
        if (len_value < 0 || len_value > TUNNEL_MAX_BODY_BYTES) {
            ESP_LOGE(TUNNEL_TAG, "Invalid body length: %.0f", len_value);
            cJSON_Delete(header);
            return ESP_FAIL;
        }
        body_len = (size_t)len_value;
    }

    uint8_t *body = NULL;
    if (body_len > 0) {
        body = malloc(body_len);
        if (!body) {
            cJSON_Delete(header);
            return ESP_ERR_NO_MEM;
        }
        err = read_exact(sock, body, body_len);
        if (err != ESP_OK) {
            free(body);
            cJSON_Delete(header);
            return err;
        }
    }

    *header_out = header;
    *body_out = body;
    *body_len_out = body_len;
    return ESP_OK;
}

static void close_tunnel_socket(tunnel_client_t *client) {
    if (client && client->sock >= 0) {
        close(client->sock);
        client->sock = -1;
    }
    if (client) {
        client->identified = false;
        client->assigned_id[0] = '\0';
    }
}

static esp_err_t connect_tunnel_socket(tunnel_client_t *client) {
    if (!client) {
        return ESP_ERR_INVALID_ARG;
    }

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", client->config.port);

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res = NULL;
    int err = getaddrinfo(client->config.host, port_str, &hints, &res);
    if (err != 0 || !res) {
        ESP_LOGE(TUNNEL_TAG, "getaddrinfo failed: %d", err);
        return ESP_FAIL;
    }

    int sock = -1;
    struct addrinfo *ptr = res;
    while (ptr) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, 0);
        if (sock < 0) {
            ptr = ptr->ai_next;
            continue;
        }
        if (connect(sock, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            break;
        }
        close(sock);
        sock = -1;
        ptr = ptr->ai_next;
    }

    freeaddrinfo(res);

    if (sock < 0) {
        ESP_LOGE(TUNNEL_TAG, "Unable to connect to %s:%d", client->config.host, client->config.port);
        return ESP_FAIL;
    }

    int keepalive = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

    client->sock = sock;
    ESP_LOGI(TUNNEL_TAG, "Connected tunnel socket to %s:%d", client->config.host, client->config.port);
    return ESP_OK;
}

static void tunnel_main_loop(tunnel_client_t *client) {
    while (1) {
        cJSON *header = NULL;
        uint8_t *body = NULL;
        size_t body_len = 0;

        esp_err_t err = read_frame(client->sock, &header, &body, &body_len);
        if (err != ESP_OK) {
            ESP_LOGW(TUNNEL_TAG, "Frame read failed, reconnecting");
            if (body) {
                free(body);
            }
            if (header) {
                cJSON_Delete(header);
            }
            break;
        }

        cJSON *type_item = cJSON_GetObjectItemCaseSensitive(header, "type");
        const char *type = cJSON_IsString(type_item) ? type_item->valuestring : NULL;

        if (!type) {
            ESP_LOGW(TUNNEL_TAG, "Received frame without type");
            cJSON_Delete(header);
            if (body) {
                free(body);
            }
            continue;
        }

        if (strcmp(type, "assign") == 0) {
            cJSON *device_id_item = cJSON_GetObjectItemCaseSensitive(header, "deviceId");
            if (cJSON_IsString(device_id_item)) {
                snprintf(client->assigned_id, sizeof(client->assigned_id), "%s", device_id_item->valuestring);
                ESP_LOGI(TUNNEL_TAG, "Assigned device ID: %s", client->assigned_id);
            }
            if (strlen(client->assigned_id) > 0) {
                cJSON *identify = cJSON_CreateObject();
                if (identify) {
                    const char *identity = (device_id[0] != '\0') ? device_id : client->assigned_id;
                    cJSON_AddStringToObject(identify, "type", "identify");
                    cJSON_AddStringToObject(identify, "deviceId", identity);
                    if (send_frame(client->sock, identify, NULL, 0) == ESP_OK) {
                        client->identified = true;
                        if (identity != client->assigned_id) {
                            snprintf(client->assigned_id, sizeof(client->assigned_id), "%s", identity);
                        }
                    }
                    cJSON_Delete(identify);
                }
            }
        } else if (strcmp(type, "ready") == 0) {
            cJSON *device_id_item = cJSON_GetObjectItemCaseSensitive(header, "deviceId");
            if (cJSON_IsString(device_id_item)) {
                snprintf(client->assigned_id, sizeof(client->assigned_id), "%s", device_id_item->valuestring);
                ESP_LOGI(TUNNEL_TAG, "Tunnel ready acknowledged by server (deviceId=%s)", client->assigned_id);
            } else {
                ESP_LOGI(TUNNEL_TAG, "Tunnel ready acknowledged by server");
            }
        } else if (strcmp(type, "ping") == 0) {
            send_simple_frame(client->sock, "pong");
        } else if (strcmp(type, "httpRequest") == 0) {
            handle_http_request_frame(client, header, body, body_len);
        } else if (strcmp(type, "abort") == 0) {
            handle_abort_frame(client, header);
        } else if (strcmp(type, "disconnect") == 0) {
            ESP_LOGW(TUNNEL_TAG, "Server requested disconnect");
            cJSON_Delete(header);
            if (body) {
                free(body);
            }
            break;
        } else {
            ESP_LOGW(TUNNEL_TAG, "Unhandled frame type: %s", type);
        }

        cJSON_Delete(header);
        if (body) {
            free(body);
        }
    }
}

static void tunnel_task(void *param) {
    tunnel_config_t config = {0};
    if (param) {
        memcpy(&config, param, sizeof(config));
        free(param);
    } else {
        load_tunnel_config(&config);
    }

    tunnel_client_t client = {
        .sock = -1,
        .identified = false,
        .assigned_id = {0},
        .config = config,
    };

    while (1) {
        if (connect_tunnel_socket(&client) != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(TUNNEL_RECONNECT_DELAY_MS));
            continue;
        }

        tunnel_main_loop(&client);
        close_tunnel_socket(&client);
        vTaskDelay(pdMS_TO_TICKS(TUNNEL_RECONNECT_DELAY_MS));
    }
}

void tunnel_start(void) {
    if (tunnel_task_started) {
        return;
    }

    tunnel_config_t *cfg = malloc(sizeof(tunnel_config_t));
    if (!cfg) {
        ESP_LOGE(TUNNEL_TAG, "Failed to allocate tunnel config");
        return;
    }
    load_tunnel_config(cfg);

    char host_copy[sizeof(cfg->host)];
    strncpy(host_copy, cfg->host, sizeof(host_copy) - 1);
    host_copy[sizeof(host_copy) - 1] = '\0';
    int port_copy = cfg->port;

    BaseType_t result = xTaskCreate(&tunnel_task, "tunnel_task", 12 * 1024, cfg, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TUNNEL_TAG, "Failed to create tunnel task");
        free(cfg);
        return;
    }

    tunnel_task_started = true;
    ESP_LOGI(TUNNEL_TAG, "Tunnel task started (connecting to %s:%d)", host_copy, port_copy);
}

