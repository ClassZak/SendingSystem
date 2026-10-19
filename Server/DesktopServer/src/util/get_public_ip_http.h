/**
 * @file    get_public_ip_http.h
 * @brief   Obtains the public IP address by querying an external
 *          HTTPS service.
 *
 * Requires libcurl (-lcurl). Unlike the CivetWeb client, libcurl
 * handles DNS, TLS, redirects and timeouts out of the box, so this is
 * the fallback when UPnP is unavailable.
 */

#pragma once

/**
 * @brief Queries https://api.ipify.org for the caller's public IP.
 *
 * @return Heap-allocated NUL-terminated string on success, NULL on
 *         network error, timeout (5 s), or malformed response.
 *
 * @note Release the result with free().
 */
char* get_public_ip_http(void);
