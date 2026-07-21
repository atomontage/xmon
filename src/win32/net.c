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
#include <stdio.h>
#include <windows.h>
#include "xmon.h"
#include "ifmib.h"

typedef DWORD (WINAPI *getiftab_func)(MIB_IFTABLE*, ULONG*, BOOL);

static HINSTANCE iphdll;
static getiftab_func get_if_table;

static MIB_IFTABLE *iftab;
static unsigned long iftab_sz;
static unsigned long prev_rx, prev_tx;


int net_init(void)
{
	unsigned int res;

	if(!(iphdll = LoadLibrary("iphlpapi.dll"))) {
		fprintf(stderr, "failed to load IP Helper library\n");
		return -1;
	}
	if(!(get_if_table = (getiftab_func)GetProcAddress(iphdll, "GetIfTable"))) {
		fprintf(stderr, "GetIfTable not found in iphlpapi.dll\n");
		goto fail;
	}
	if((res = get_if_table(0, &iftab_sz, 0)) != ERROR_INSUFFICIENT_BUFFER) {
		fprintf(stderr, "failed to query interface table size\n");
		goto fail;
	}
	if(!(iftab = malloc(iftab_sz))) {
		fprintf(stderr, "failed to allocate interface table (%lu bytes)\n", iftab_sz);
		goto fail;
	}

	net_update();
	return 0;

fail:
	if(iphdll) {
		FreeLibrary(iphdll);
	}
	free(iftab);
	return -1;
}

void net_update(void)
{
	void *newbuf;
	unsigned long newsz, cur_rx, cur_tx;
	unsigned int i, res;

	while((res = get_if_table(iftab, &iftab_sz, 0)) != 0) {
		if(res != ERROR_INSUFFICIENT_BUFFER) {
			fprintf(stderr, "failed to retreive interface table\n");
			return;
		}
		newsz = iftab_sz << 1;
		if(!(newbuf = malloc(newsz))) {
			fprintf(stderr, "failed to allocate interface table (%lu bytes)\n", newsz);
			return;
		}
		free(iftab);
		iftab = newbuf;
		iftab_sz = newsz;
	}

	cur_rx = cur_tx = 0;
	for(i=0; i<iftab->dwNumEntries; i++) {
		MIB_IFROW *row = iftab->table + i;
		cur_rx += row->dwInOctets;
		cur_tx += row->dwOutOctets;
	}

	smon.net_rx = prev_rx ? cur_rx - prev_rx : 0;
	smon.net_tx = prev_tx ? cur_tx - prev_tx : 0;
	prev_rx = cur_rx;
	prev_tx = cur_tx;
}
