/*
xmon - a minimal lightweight cross-platform system monitoring utility
Copyright (C) 2025-2026  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/route.h>
#include "xmon.h"
#include "options.h"

static int fetch_net_single(int ifidx, unsigned long *rx_out, unsigned long *tx_out);
static int fetch_net(unsigned long *rx_out, unsigned long *tx_out);

static unsigned long prev_rx, prev_tx;
static unsigned int ifidx;

int net_init(void)
{
	if(opt.net.ifname) {
		if((ifidx = if_nametoindex(opt.net.ifname)) == 0) {
			fprintf(stderr, "failed to find network interface: %s\n", opt.net.ifname);
			return -1;
		}
	}
	return 0;
}

void net_update(void)
{
	unsigned long cur_rx = 0, cur_tx = 0;

	if(ifidx > 0) {
		/* Only retrieve byte counters for user-selected interface */
		fetch_net_single(ifidx, &cur_rx, &cur_tx);
	} else {
		/* Retrieve and aggregate byte counters for all interfaces */
		fetch_net(&cur_rx, &cur_tx);
	}

	smon.net_rx = prev_rx ? cur_rx - prev_rx : 0;
	smon.net_tx = prev_tx ? cur_tx - prev_tx : 0;

	prev_rx = cur_rx;
	prev_tx = cur_tx;
}

static int fetch_net_single(int ifidx, unsigned long *rx_out, unsigned long *tx_out)
{
	struct ifmibdata ifmd;
	size_t ifmd_len;
	int mib[6];

	mib[0] = CTL_NET;
	mib[1] = PF_LINK;
	mib[2] = NETLINK_GENERIC;
	mib[3] = IFMIB_IFDATA;
	mib[4] = ifidx;
	mib[5] = IFDATA_GENERAL;

	ifmd_len = sizeof ifmd;
	if(sysctl(mib, 6, &ifmd, &ifmd_len, NULL, 0) < 0) {
		perror("sysctl(IFMID_IFDATA)");
		return -1;
	}

	*rx_out = ifmd.ifmd_data.ifi_ibytes;
	*tx_out = ifmd.ifmd_data.ifi_obytes;
	return 0;
}

static int fetch_net(unsigned long *rx_out, unsigned long *tx_out)
{
	int mib[6];
	size_t len;
	unsigned char *buf, *msg;
	unsigned long rx = 0, tx = 0;

	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;
	mib[3] = 0;
	mib[4] = NET_RT_IFLIST2;
	mib[5] = 0;

	if(sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
		perror("sysctl(NET_RT_IFLIST2, NULL)");
		return -1;
	}

	if(!(buf = calloc(1, len))) {
		fprintf(stderr, "failed to allocate interface list buffer\n");
		return -1;
	}

	if(sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
		perror("sysctl(NET_RT_IFLIST2)");
		free(buf);
		return -1;
	}

	msg = buf;
	while(msg < (buf + len)) {
		struct if_msghdr *msghdr = (struct if_msghdr*)msg;
		if(msghdr->ifm_type == RTM_IFINFO2) {
			unsigned long rxtemp, txtemp;
			fetch_net_single(msghdr->ifm_index, &rxtemp, &txtemp);
			rx += rxtemp;
			tx += txtemp;
		}
		msg += msghdr->ifm_msglen;
	}

	*rx_out = rx;
	*tx_out = tx;
	free(buf);

	return 0;
}

