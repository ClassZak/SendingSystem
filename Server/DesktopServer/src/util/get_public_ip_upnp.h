/**
 * @file    get_public_ip_upnp.h
 * @brief   Obtains the router's WAN IP address via UPnP IGD.
 *
 * Requires the miniupnpc library (-lminiupnpc). If the router does not
 * support UPnP, or has it disabled, the function returns NULL. This is
 * expected in corporate networks — the caller should fall back to
 * get_public_ip_http().
 */

#pragma once

/**
 * @brief Asks the local UPnP Internet Gateway Device for its external
 *        (WAN) IP address.
 *
 * @return Heap-allocated NUL-terminated string on success, NULL if no
 *         UPnP gateway was found or the query failed.
 *
 * @note Release the result with free().
 */
char* get_public_ip_upnp(void);
