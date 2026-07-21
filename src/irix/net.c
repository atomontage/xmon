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
#ifndef NET_H_
#define NET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "xmon.h"
#include "options.h"

static int sock;
static char **ifnames;
static int num_ifs;
static unsigned long prev_rx, prev_tx;

int net_init(void)
{
	unsigned int i, bufsz, count, max_count;
	struct ifconf ifc;
	struct ifreq *ifbuf = 0;
	struct ifreq *ifr, *ifr_end;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "failed to create socket: %s\n", strerror(errno));
		return -1;
	}

	max_count = 8;
	do {
		free(ifbuf);
		max_count <<= 1;
		bufsz = max_count * sizeof *ifbuf;
		if(!(ifbuf = malloc(bufsz))) {
			fprintf(stderr, "failed to allocate space for %d ifreq structures\n", max_count);
			goto fail;
		}
		ifc.ifc_len = bufsz;
		ifc.ifc_buf = (char*)ifbuf;

		if(ioctl(sock, SIOCGIFCONF, &ifc) == -1 && errno != EINVAL) {
			fprintf(stderr, "failed to get interface config info: %s\n", strerror(errno));
			goto fail;
		}
	} while(ifc.ifc_len >= bufsz);

	count = ifc.ifc_len / sizeof *ifr;
	if(!(ifnames = malloc(count * sizeof *ifnames))) {
		fprintf(stderr, "failed to allocate interface names array (%u)\n", count);
		goto fail;
	}

	num_ifs = 0;
	ifr = ifbuf;
	ifr_end = (struct ifreq*)((char*)ifbuf + ifc.ifc_len);
	while(ifr < ifr_end) {
		if(opt.net.ifname) {
			if(strcmp(ifr->ifr_name, opt.net.ifname) != 0) {
				ifr++;
				continue;
			}
		} else {
			if(ifr->ifr_addr.sa_family != AF_INET && ifr->ifr_addr.sa_family != AF_INET6) {
				ifr++;
				continue;
			}
		}
		if(!(ifnames[num_ifs++] = strdup(ifr->ifr_name))) {
			fprintf(stderr, "failed to allocate memory for interface name: %s\n", ifr->ifr_name);
			goto fail;
		}
		ifr++;
	}

	if(!num_ifs) {
		if(opt.net.ifname) {
			fprintf(stderr, "interface %s not found\n", opt.net.ifname);
		} else {
			fprintf(stderr, "no network interfaces found\n");
		}
		goto fail;
	}

	free(ifbuf);
	return 0;

fail:
	close(sock);
	free(ifbuf);
	if(ifnames) {
		for(i=0; i<count; i++) {
			free(ifnames[i]);
		}
		free(ifnames);
	}
	return -1;
}

void net_update(void)
{
	int i;
	struct ifreq ifr;
	unsigned long cur_rx, cur_tx;

	cur_rx = cur_tx = 0;

	for(i=0; i<num_ifs; i++) {
		strcpy(ifr.ifr_name, ifnames[i]);
		if(ioctl(sock, SIOCGIFSTATS, &ifr) == -1) {
			continue;
		}
		cur_rx += ifr.ifr_stats.ifs_ipackets;
		cur_tx += ifr.ifr_stats.ifs_opackets;
	}

	smon.net_rx = prev_rx ? cur_rx - prev_rx : 0;
	smon.net_tx = prev_tx ? cur_tx - prev_tx : 0;
	prev_rx = cur_rx;
	prev_tx = cur_tx;
}

#endif	/* NET_H_ */
