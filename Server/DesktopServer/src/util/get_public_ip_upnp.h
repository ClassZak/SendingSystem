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
