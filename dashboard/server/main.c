// main.c - TrashBot Dashboard host server in C (CivetWeb + cJSON)
// Serves website from ../web
// REST:
//   POST /vision/detection
//   POST /arm/status
//   GET  /state
// WS:
//   /ws  (broadcasts state updates to all connected clients)

#include "civetweb.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static long long now_ms(void) { return (long long)GetTickCount64(); }
#else
#include <sys/time.h>
static long long now_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL;
}
#endif

// ---------------- Shared state ----------------
static cJSON *g_state = NULL;
static struct mg_connection *g_ws_clients[64];
static int g_ws_count = 0;

static void ws_add_client(struct mg_connection *conn) {
  if (g_ws_count < (int)(sizeof(g_ws_clients)/sizeof(g_ws_clients[0]))) {
    g_ws_clients[g_ws_count++] = conn;
  }
}

static void ws_remove_client(struct mg_connection *conn) {
  for (int i = 0; i < g_ws_count; i++) {
    if (g_ws_clients[i] == conn) {
      g_ws_clients[i] = g_ws_clients[g_ws_count - 1];
      g_ws_count--;
      return;
    }
  }
}

static void state_init(void) {
  if (g_state) cJSON_Delete(g_state);
  g_state = cJSON_CreateObject();

  cJSON_AddNumberToObject(g_state, "last_update_ms", (double)now_ms());

  cJSON *vision = cJSON_AddObjectToObject(g_state, "vision");
  cJSON_AddBoolToObject(vision, "online", 0);
  cJSON_AddNullToObject(vision, "last_seen_ms");
  cJSON_AddNullToObject(vision, "latest");

  cJSON *arm = cJSON_AddObjectToObject(g_state, "arm");
  cJSON_AddBoolToObject(arm, "online", 0);
  cJSON_AddNullToObject(arm, "last_seen_ms");
  cJSON_AddStringToObject(arm, "status", "unknown");
  cJSON_AddNullToObject(arm, "latest");

  cJSON *counts = cJSON_AddObjectToObject(g_state, "counts");
  cJSON_AddNumberToObject(counts, "total", 0);
  cJSON_AddNumberToObject(counts, "recyclable", 0);
  cJSON_AddNumberToObject(counts, "trash", 0);
  cJSON_AddNumberToObject(counts, "errors", 0);
}

static void state_touch(void) {
  cJSON_ReplaceItemInObject(g_state, "last_update_ms", cJSON_CreateNumber((double)now_ms()));
}

static char *json_stringify_dup(const cJSON *obj) {
  // cJSON_PrintUnformatted allocates; caller must free()
  return cJSON_PrintUnformatted(obj);
}

static void ws_broadcast_state(const char *event_type, const cJSON *payload_or_null) {
  if (g_ws_count <= 0) return;

  cJSON *msg = cJSON_CreateObject();
  cJSON_AddStringToObject(msg, "type", event_type);
  cJSON_AddNumberToObject(msg, "ts_ms", (double)now_ms());

  // Copy state (stringify + parse is simplest; state is small)
  char *state_str = json_stringify_dup(g_state);
  cJSON *state_copy = cJSON_Parse(state_str);
  free(state_str);

  cJSON_AddItemToObject(msg, "state", state_copy);

  if (payload_or_null) {
    // deep copy payload
    char *p = json_stringify_dup(payload_or_null);
    cJSON_AddItemToObject(msg, "payload", cJSON_Parse(p));
    free(p);
  } else {
    cJSON_AddNullToObject(msg, "payload");
  }

  char *out = json_stringify_dup(msg);
  cJSON_Delete(msg);

  for (int i = 0; i < g_ws_count; i++) {
    // mg_websocket_write(conn, opcode, data, len)
    mg_websocket_write(g_ws_clients[i], WEBSOCKET_OPCODE_TEXT, out, strlen(out));
  }
  free(out);
}

// --------------- Helpers ----------------
static int read_request_body(struct mg_connection *conn, char *buf, size_t buf_sz) {
  // CivetWeb: content-length available via mg_get_request_info(conn)->content_length
  const struct mg_request_info *ri = mg_get_request_info(conn);
  long long cl = ri->content_length;
  if (cl <= 0 || (size_t)cl >= buf_sz) return 0;

  int n = mg_read(conn, buf, (size_t)cl);
  if (n <= 0) return 0;

  buf[n] = '\0';
  return n;
}

static void send_json(struct mg_connection *conn, int status, const char *json_body) {
  mg_printf(conn,
            "HTTP/1.1 %d OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            status, strlen(json_body), json_body);
}

// --------------- REST handlers ----------------
static int handle_get_state(struct mg_connection *conn, void *cbdata) {
  (void)cbdata;
  char *s = json_stringify_dup(g_state);
  send_json(conn, 200, s);
  free(s);
  return 1; // handled
}

static int handle_post_vision(struct mg_connection *conn, void *cbdata) {
  (void)cbdata;
  char body[4096];
  if (!read_request_body(conn, body, sizeof(body) - 1)) {
    send_json(conn, 400, "{\"ok\":false,\"error\":\"bad body\"}");
    return 1;
  }

  cJSON *d = cJSON_Parse(body);
  if (!d) {
    send_json(conn, 400, "{\"ok\":false,\"error\":\"invalid json\"}");
    return 1;
  }

  // Update state
  state_touch();
  cJSON *vision = cJSON_GetObjectItem(g_state, "vision");
  cJSON_ReplaceItemInObject(vision, "online", cJSON_CreateBool(1));
  cJSON_ReplaceItemInObject(vision, "last_seen_ms", cJSON_CreateNumber((double)now_ms()));

  // Replace latest
  cJSON_ReplaceItemInObject(vision, "latest", d); // d now owned by state

  // Update counts: total++, recyclable/trash++
  cJSON *counts = cJSON_GetObjectItem(g_state, "counts");
  int total = cJSON_GetObjectItem(counts, "total")->valueint;
  cJSON_ReplaceItemInObject(counts, "total", cJSON_CreateNumber(total + 1));

  cJSON *recyclable = cJSON_GetObjectItem(vision, "latest");
  cJSON *is_rec = cJSON_GetObjectItem(recyclable, "recyclable");
  if (is_rec && cJSON_IsBool(is_rec) && cJSON_IsTrue(is_rec)) {
    int r = cJSON_GetObjectItem(counts, "recyclable")->valueint;
    cJSON_ReplaceItemInObject(counts, "recyclable", cJSON_CreateNumber(r + 1));
  } else {
    int t = cJSON_GetObjectItem(counts, "trash")->valueint;
    cJSON_ReplaceItemInObject(counts, "trash", cJSON_CreateNumber(t + 1));
  }

  ws_broadcast_state("vision_update", cJSON_GetObjectItem(vision, "latest"));
  send_json(conn, 200, "{\"ok\":true}");
  return 1;
}

static int handle_post_arm(struct mg_connection *conn, void *cbdata) {
  (void)cbdata;
  char body[2048];
  if (!read_request_body(conn, body, sizeof(body) - 1)) {
    send_json(conn, 400, "{\"ok\":false,\"error\":\"bad body\"}");
    return 1;
  }

  cJSON *s = cJSON_Parse(body);
  if (!s) {
    send_json(conn, 400, "{\"ok\":false,\"error\":\"invalid json\"}");
    return 1;
  }

  state_touch();
  cJSON *arm = cJSON_GetObjectItem(g_state, "arm");
  cJSON_ReplaceItemInObject(arm, "online", cJSON_CreateBool(1));
  cJSON_ReplaceItemInObject(arm, "last_seen_ms", cJSON_CreateNumber((double)now_ms()));

  cJSON *status = cJSON_GetObjectItem(s, "status");
  if (status && cJSON_IsString(status)) {
    cJSON_ReplaceItemInObject(arm, "status", cJSON_CreateString(status->valuestring));
  }

  cJSON_ReplaceItemInObject(arm, "latest", s); // s now owned by state

  // if error, increment errors
  cJSON *counts = cJSON_GetObjectItem(g_state, "counts");
  if (status && cJSON_IsString(status) && strcmp(status->valuestring, "error") == 0) {
    int e = cJSON_GetObjectItem(counts, "errors")->valueint;
    cJSON_ReplaceItemInObject(counts, "errors", cJSON_CreateNumber(e + 1));
  }

  ws_broadcast_state("arm_update", cJSON_GetObjectItem(arm, "latest"));
  send_json(conn, 200, "{\"ok\":true}");
  return 1;
}

// --------------- WebSocket handlers ----------------
static int ws_connect_handler(const struct mg_connection *conn, void *cbdata) {
  (void)conn; (void)cbdata;
  // Returning 0 accepts connection
  return 0;
}

static void ws_ready_handler(struct mg_connection *conn, void *cbdata) {
  (void)cbdata;
  ws_add_client(conn);

  // Send initial snapshot
  cJSON *msg = cJSON_CreateObject();
  cJSON_AddStringToObject(msg, "type", "init");
  cJSON_AddNumberToObject(msg, "ts_ms", (double)now_ms());

  char *state_str = json_stringify_dup(g_state);
  cJSON_AddItemToObject(msg, "state", cJSON_Parse(state_str));
  free(state_str);

  char *out = json_stringify_dup(msg);
  cJSON_Delete(msg);

  mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, out, strlen(out));
  free(out);
}

static int ws_data_handler(struct mg_connection *conn, int flags, char *data, size_t data_len, void *cbdata) {
  (void)conn; (void)flags; (void)data; (void)data_len; (void)cbdata;
  // We don't need client->server messages; keep-alives are fine
  return 1;
}

static void ws_close_handler(const struct mg_connection *conn, void *cbdata) {
  (void)cbdata;
  ws_remove_client((struct mg_connection *)conn);
}

// --------------- main ----------------
int main(void) {
  state_init();

  const char *options[] = {
    "listening_ports", "8080",
    // Serve files from ../web
    "document_root", "../web",
    "enable_directory_listing", "no",
    0
  };

  struct mg_callbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));

  struct mg_context *ctx = mg_start(&callbacks, 0, options);
  if (!ctx) {
    fprintf(stderr, "Failed to start server.\n");
    return 1;
  }

  // REST routes
  mg_set_request_handler(ctx, "/state", handle_get_state, 0);
  mg_set_request_handler(ctx, "/vision/detection", handle_post_vision, 0);
  mg_set_request_handler(ctx, "/arm/status", handle_post_arm, 0);

  // WebSocket route
  mg_set_websocket_handler(ctx, "/ws",
                           ws_connect_handler,
                           ws_ready_handler,
                           ws_data_handler,
                           ws_close_handler,
                           0);

  printf("TrashBot Dashboard Server running:\n");
  printf("  Website: http://localhost:8080/\n");
  printf("  State:   http://localhost:8080/state\n");
  printf("  WS:      ws://localhost:8080/ws\n");
  printf("Press Enter to quit.\n");
  getchar();

  mg_stop(ctx);
  cJSON_Delete(g_state);
  return 0;
}
