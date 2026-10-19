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
 * @file    get_server_ip_addresses.h
 * @brief   Returns one IP address per distinct subnet the host has an
 *          interface on, optionally with the subnet prefix length.
 */

#pragma once

/**
 * @brief Controls whether the subnet prefix length is appended to each
 *        returned address.
 */
typedef enum {
	SERVER_IP_PLAIN     = 0,   /**< "192.168.1.42"           */
	SERVER_IP_WITH_MASK = 1    /**< "192.168.1.42/24"        */
} server_ip_mask_mode;

/**
 * @brief Returns one IPv4 or IPv6 address for every distinct subnet
 *        attached to the host.
 *
 * Loopback counts as a subnet (127.0.0.0/8, ::1/128). Link-local,
 * private (RFC 1918) and public subnets are all included without
 * distinction. If an interface has several addresses in the same
 * subnet, only the first one is returned.
 *
 * @param[out] count     Receives the number of returned entries. Must
 *                       not be NULL. Set to 0 on failure.
 * @param[in]  mask_mode SERVER_IP_PLAIN returns bare addresses;
 *                       SERVER_IP_WITH_MASK appends "/<prefix>" (CIDR).
 *
 * @return Heap-allocated array of `*count` NUL-terminated strings, or
 *         NULL if no addresses were found or allocation failed.
 *
 * @note Owns the array and every string: release with
 *       free_server_ip_addresses().
 */
char** get_server_ip_addresses(int* count, server_ip_mask_mode mask_mode);

/**
 * @brief Frees an array returned by get_server_ip_addresses().
 *
 * @param ips   Array to free. May be NULL.
 * @param count Number of entries in @p ips.
 */
void free_server_ip_addresses(char** ips, int count);
