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
 * @file    get_public_ip_http.c
 * @brief   Public IP discovery via an external HTTPS service.
 *
 * ── How it works ──────────────────────────────────────────────────────
 *
 * 1. curl_easy_init() creates a handle.
 * 2. CURLOPT_URL points to a plain-text endpoint that echoes the
 *    caller's public IP (api.ipify.org, ifconfig.me, ipinfo.io/ip).
 * 3. CURLOPT_WRITEFUNCTION accumulates the body into a heap buffer.
 * 4. CURLOPT_TIMEOUT bounds the whole transfer to 5 seconds.
 * 5. curl_easy_perform() runs the request.
 *
 * api.ipify.org was chosen because it:
 *   - speaks HTTPS with a valid certificate (no TrustAll needed),
 *   - returns the IP as plain text (no JSON parsing),
 *   - answers over IPv4 and IPv6.
 *
 * Other equivalent endpoints (swap CURLOPT_URL):
 *   https://ifconfig.me/ip
 *   https://ipinfo.io/ip
 *   https://icanhazip.com
 */

#include "get_public_ip_http.h"

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>


static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total = size * nmemb;
	char** buf = (char**)userp;
	size_t cur_len = *buf != NULL ? strlen(*buf) : 0;

	char* tmp = realloc(*buf, cur_len + total + 1);
	if (tmp == NULL) {
		return 0;
	}

	memcpy(tmp + cur_len, contents, total);
	tmp[cur_len + total] = '\0';
	*buf = tmp;

	return total;
}




char* get_public_ip_http(void) {
	CURL* curl = curl_easy_init();
	if (curl == NULL) {
		return NULL;
	}

	char* buffer = malloc(1);
	if (buffer == NULL) {
		curl_easy_cleanup(curl);

		return NULL;
	}
	buffer[0] = '\0';

	curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "SendingSystem/1.0");
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		free(buffer);

		return NULL;
	}

	size_t len = strlen(buffer);
	while (len > 0 &&
	       (buffer[len - 1] == '\n' ||
	        buffer[len - 1] == '\r' ||
	        buffer[len - 1] == ' ')) {
		buffer[--len] = '\0';
	}

	if (len == 0) {
		free(buffer);

		return NULL;
	}

	return buffer;
}
