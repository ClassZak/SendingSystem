#pragma once
#include <civetweb.h>

/* Sends {"type": ..., "data": ...} and closes the connection.
 * status: 200, 400, 405, 500, etc. */
void http_send_json(struct mg_connection *conn, int status,
					const char *type, const char *data);

/* Shortcut: 4xx/5xx with text in the "data" field */
void http_send_error(struct mg_connection *conn, int status, const char *msg);
