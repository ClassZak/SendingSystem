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
 * UPnP IGD discovery of the public IP address.
 *
 * ── How it works ──────────────────────────────────────────────────────
 *
 * 1. upnpDiscover() sends SSDP M-SEARCH on the local network and
 *    collects replies from UPnP devices for a given timeout.
 *
 * 2. UPNP_GetValidIGD() picks the device that actually offers the
 *    "WANIPConnection" or "WANPPPConnection" service (i.e. the router)
 *    and returns its control URL and service type.
 *
 * 3. UPNP_GetExternalIPAddress() asks the router for its WAN address
 *    via SOAP.
 *
 * 4. FreeUPNPUrls() and freeUPNPDevlist() release the discovery data.
 *
 * The router must have UPnP enabled. Many corporate networks disable
 * it; in that case the caller should fall back to an HTTP service.
 */

#include "get_public_ip_upnp.h"

#include <stdlib.h>
#include <string.h>

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>


char* get_public_ip_upnp(void) {
	struct UPNPDev* devlist = upnpDiscover(2000, NULL, NULL, 0, 0, 2, NULL);
	if (devlist == NULL) {
		return NULL;
	}

	struct UPNPUrls urls;
	struct IGDdatas data;
	char lanaddr[64];
	char wanaddr[40];

	int r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr), NULL, 0);
	if (r <= 0) {
		freeUPNPDevlist(devlist);

		return NULL;
	}

	char* result = NULL;

	if (UPNP_GetExternalIPAddress(urls.controlURL,
	                              data.first.servicetype,
	                              wanaddr) == 0 &&
	    wanaddr[0] != '\0') {
		result = strdup(wanaddr);
	}

	FreeUPNPUrls(&urls);
	freeUPNPDevlist(devlist);

	return result;
}
