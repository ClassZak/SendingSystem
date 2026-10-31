/*
 * Sending System
 * Copyright (C) 2025-2026 ClassZak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * Implementation of GET /api/openapi.json.
 */

#include "api_openapi.h"
#include "http_json.h"
#include "../util/config.h"


#include <cJSON.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>




/**
 * One row in the public-API catalogue.
 */
typedef struct {
	const char* request_method;
	const char* endpoint_path;
	const char* summary;
	const char* request_content_type;
	const char* request_schema_json;
	const char* success_schema_reference;
} ApiEndpoint;




/**
 * Public API catalogue. Add a row whenever a new handler is registered
 * in Server.c.
 */
static const ApiEndpoint k_endpoints[] = {
	{
		"POST",
		"/api/send",
		"Send a text message. If saveToPath is set and non-empty, the "
		"server also writes it to SAVE_DIR/<sanitized_name>.",
		"application/json",
		"{"
			"\"type\":\"object\","
			"\"properties\":{"
				"\"data\":{\"type\":\"string\",\"minLength\":1},"
				"\"saveToPath\":{\"type\":\"string\"}"
			"},"
			"\"required\":[\"data\"]"
		"}",
		"#/components/schemas/MessageResponse"
	},
	{
		"POST",
		"/api/upload",
		"Upload a file as multipart/form-data. The file is streamed to "
		"SAVE_DIR/<sanitized_name>; memory usage is independent of size.",
		"multipart/form-data",
		"{"
			"\"type\":\"object\","
			"\"properties\":{"
				"\"file\":{\"type\":\"string\",\"format\":\"binary\"}"
			"},"
			"\"required\":[\"file\"]"
		"}",
		"#/components/schemas/MessageResponse"
	},
	{
		"GET",
		"/api/openapi.json",
		"This document: an OpenAPI 3.1 description of the public API.",
		NULL,
		NULL,
		NULL
	}
};




static const char* k_error_schema_reference =
	"#/components/schemas/ErrorResponse";




/**
 * Appends one HTTP response object to a `responses` map.
 *
 * @param responses        (cJSON*, Required) — object being built.
 * @param status_code      (const char*, Required) — "200", "400", ...
 * @param description      (const char*, Required) — short human text.
 * @param schema_reference (const char*, Optional) — JSON pointer; NULL
 *                         means "no typed body".
 */
static void add_response(cJSON* responses,
                         const char* status_code,
                         const char* description,
                         const char* schema_reference) {
	cJSON* response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, "description", description);

	if (schema_reference) {
		cJSON* content = cJSON_AddObjectToObject(response, "content");
		cJSON* media_type = cJSON_AddObjectToObject(
			content, "application/json");
		cJSON* schema = cJSON_AddObjectToObject(media_type, "schema");
		cJSON_AddStringToObject(schema, "$ref", schema_reference);
	}

	cJSON_AddItemToObject(responses, status_code, response);
}




/**
 * Builds the OpenAPI "operation" object for one endpoint.
 *
 * @param endpoint (const ApiEndpoint*, Required) — catalogue row.
 * @return (cJSON*) — heap object; caller attaches it under paths.
 */
static cJSON* build_operation(const ApiEndpoint* endpoint) {
	cJSON* operation = cJSON_CreateObject();
	cJSON_AddStringToObject(operation, "summary", endpoint->summary);

	if (endpoint->request_content_type) {
		cJSON* request_body = cJSON_AddObjectToObject(
			operation, "requestBody");
		cJSON_AddBoolToObject(request_body, "required", 1);

		cJSON* content = cJSON_AddObjectToObject(
			request_body, "content");
		cJSON* media_type = cJSON_AddObjectToObject(
			content, endpoint->request_content_type);

		if (endpoint->request_schema_json) {
			cJSON* schema = cJSON_Parse(endpoint->request_schema_json);
			if (schema) {
				cJSON_AddItemToObject(media_type, "schema", schema);
			}
		}
	}

	cJSON* responses = cJSON_AddObjectToObject(operation, "responses");
	add_response(responses, "200", "Success",
	             endpoint->success_schema_reference);
	add_response(responses, "400", "Bad request",
	             k_error_schema_reference);
	add_response(responses, "405", "Method not allowed",
	             k_error_schema_reference);
	add_response(responses, "500", "Internal error",
	             k_error_schema_reference);

	return operation;
}




/**
 * Fills `components.schemas` with the shared response objects.
 *
 * @param root (cJSON*, Required) — top-level OpenAPI document.
 */
static void add_components(cJSON* root) {
	cJSON* components = cJSON_AddObjectToObject(root, "components");
	cJSON* schemas = cJSON_AddObjectToObject(components, "schemas");

	/* MessageResponse */
	cJSON* message_schema = cJSON_CreateObject();
	cJSON_AddStringToObject(message_schema, "type", "object");
	cJSON* message_properties = cJSON_AddObjectToObject(
		message_schema, "properties");

	cJSON* message_type = cJSON_AddObjectToObject(
		message_properties, "type");
	cJSON_AddStringToObject(message_type, "type", "string");
	cJSON_AddStringToObject(message_type, "example", "message");

	cJSON* message_data = cJSON_AddObjectToObject(
		message_properties, "data");
	cJSON_AddStringToObject(message_data, "type", "string");
	cJSON_AddStringToObject(message_data, "example", "OK");

	cJSON* message_required = cJSON_AddArrayToObject(
		message_schema, "required");
	cJSON_AddItemToArray(message_required,
	                     cJSON_CreateString("type"));
	cJSON_AddItemToArray(message_required,
	                     cJSON_CreateString("data"));

	cJSON_AddItemToObject(schemas, "MessageResponse", message_schema);


	/* ErrorResponse */
	cJSON* error_schema = cJSON_CreateObject();
	cJSON_AddStringToObject(error_schema, "type", "object");
	cJSON* error_properties = cJSON_AddObjectToObject(
		error_schema, "properties");

	cJSON* error_type = cJSON_AddObjectToObject(
		error_properties, "type");
	cJSON_AddStringToObject(error_type, "type", "string");
	cJSON_AddStringToObject(error_type, "example", "error");

	cJSON* error_data = cJSON_AddObjectToObject(
		error_properties, "data");
	cJSON_AddStringToObject(error_data, "type", "string");
	cJSON_AddStringToObject(error_data, "example",
	                        "Invalid Content-Length");

	cJSON* error_required = cJSON_AddArrayToObject(
		error_schema, "required");
	cJSON_AddItemToArray(error_required,
	                     cJSON_CreateString("type"));
	cJSON_AddItemToArray(error_required,
	                     cJSON_CreateString("data"));

	cJSON_AddItemToObject(schemas, "ErrorResponse", error_schema);
}




/**
 * Builds the `servers[0].url` value from the request.
 *
 * CivetWeb does not expose the local socket address on
 * mg_request_info (local_addr is a Ceph patch, not upstream).
 * The portable way is the standard Host header, which every HTTP/1.1
 * client sends.
 *
 * @param connection      (struct mg_connection*, Required)
 * @param buffer          (char*, Required) — output buffer.
 * @param buffer_capacity (size_t, Required) — capacity of buffer.
 */
static void build_server_url(struct mg_connection* connection,
                             char* buffer,
                             size_t buffer_capacity) {
	const char* host_header = mg_get_header(connection, "Host");
	const char* scheme = SSL_ENCRYPT ? "https" : "http";

	if (host_header && host_header[0] != '\0') {
		snprintf(buffer, buffer_capacity, "%s://%s",
		         scheme, host_header);
	} else {
		/* HTTP/1.0 or broken client: fall back to loopback. */
		snprintf(buffer, buffer_capacity, "%s://127.0.0.1:%d",
		         scheme, CONNECTION_PORT);
	}
}




int api_openapi_handler(struct mg_connection* connection,
                        void* callback_data) {
	const struct mg_request_info* request_info =
		mg_get_request_info(connection);

	const bool is_get  =
		strcmp(request_info->request_method, "GET")  == 0;
	const bool is_head =
		strcmp(request_info->request_method, "HEAD") == 0;
	if (!is_get && !is_head) {
		http_send_error(connection, 405, "GET required");
		return 1;
	}


	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "openapi", "3.1.0");

	cJSON* info = cJSON_AddObjectToObject(root, "info");
	cJSON_AddStringToObject(info, "title", "SendingSystem API");
	cJSON_AddStringToObject(info, "version", "1.0.0");
	cJSON_AddStringToObject(info, "description",
	                        "Public HTTP API of the SendingSystem server. "
	                        "Request and response bodies are JSON unless "
	                        "an operation says otherwise.");


	char server_url[128];
	build_server_url(connection, server_url, sizeof(server_url));

	cJSON* servers = cJSON_AddArrayToObject(root, "servers");
	cJSON* server_entry = cJSON_CreateObject();
	cJSON_AddStringToObject(server_entry, "url", server_url);
	cJSON_AddItemToArray(servers, server_entry);


	cJSON* paths = cJSON_AddObjectToObject(root, "paths");
	size_t endpoint_count =
		sizeof(k_endpoints) / sizeof(k_endpoints[0]);

	for (size_t index = 0; index < endpoint_count; index++) {
		const ApiEndpoint* endpoint = &k_endpoints[index];

		cJSON* path_item = cJSON_GetObjectItem(
			paths, endpoint->endpoint_path);
		if (!path_item) {
			path_item = cJSON_AddObjectToObject(
				paths, endpoint->endpoint_path);
		}

		const char* operation_name =
			(endpoint->request_method[0] == 'G') ? "get" : "post";
		cJSON_AddItemToObject(path_item, operation_name,
		                      build_operation(endpoint));
	}

	add_components(root);


	char* json = cJSON_PrintUnformatted(root);
	int json_length = json ? (int)strlen(json) : 0;

	mg_printf(connection,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Type: application/vnd.oai.openapi+json; charset=utf-8\r\n"
	          "Cache-Control: max-age=60\r\n"
	          "Content-Length: %d\r\n"
	          "Connection: close\r\n\r\n",
	          json_length);

	if (is_get && json_length > 0) {
		mg_write(connection, json, json_length);
	}

	free(json);
	cJSON_Delete(root);
	return 1;
}

