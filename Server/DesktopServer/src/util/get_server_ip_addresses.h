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
 * Returns one IP address per distinct subnet attached to the host.
 *
 * Loopback, link-local, private (RFC 1918) and public subnets are all
 * included. When two addresses share a subnet, only the first is
 * returned. Each entry is optionally suffixed with "/<prefix>".
 */
#pragma once




/**
 * Controls whether the subnet prefix length is appended to each
 * returned address.
 */
typedef enum {
	SERVER_IP_PLAIN     = 0,   /**< "192.168.1.42"     */
	SERVER_IP_WITH_MASK = 1    /**< "192.168.1.42/24"  */
} server_ip_mask_mode;




/**
 * Returns one IPv4 or IPv6 address for every distinct subnet.
 *
 * If an interface has several addresses in the same subnet, only the
 * first one is returned.
 *
 * @param count     (int*, Required) — receives the number of returned
 *                  entries; set to 0 on failure.
 * @param mask_mode (server_ip_mask_mode, Required) — SERVER_IP_PLAIN
 *                  for bare addresses, SERVER_IP_WITH_MASK to append
 *                  "/<prefix>".
 * @return (char**) — heap array of *count NUL-terminated strings, or
 *         NULL. Caller owns the array and every string; release with
 *         free_server_ip_addresses.
 */
char** get_server_ip_addresses(int* count, server_ip_mask_mode mask_mode);




/**
 * Frees an array returned by get_server_ip_addresses.
 *
 * @param ips   (char**, Optional) — array; NULL is safe.
 * @param count (int, Required) — number of entries in @p ips.
 */
void free_server_ip_addresses(char** ips, int count);
