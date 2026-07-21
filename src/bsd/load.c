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
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include "xmon.h"

static int shift;

int load_init(void)
{
	struct loadavg la;
	size_t len = sizeof la;

	if(sysctlbyname("vm.loadavg", &la, &len, 0, 0) == -1) {
		fprintf(stderr, "failed to get load average\n");
		return -1;
	}

	shift = 0;
	while(la.fscale > 1) {
		la.fscale >>= 1;
		shift++;
	}
	shift -= 10;	/* we store load in 12.10 fixed point */

	return 0;
}

void load_update(void)
{
	struct loadavg la;
	size_t len = sizeof la;

	sysctlbyname("vm.loadavg", &la, &len, 0, 0);

	if(shift == 0) {
		smon.loadavg[0] = la.ldavg[0];
		smon.loadavg[1] = la.ldavg[1];
		smon.loadavg[2] = la.ldavg[2];
	} else if(shift > 0) {
		smon.loadavg[0] = la.ldavg[0] << shift;
		smon.loadavg[1] = la.ldavg[1] << shift;
		smon.loadavg[2] = la.ldavg[2] << shift;
	} else {
		smon.loadavg[0] = la.ldavg[0] >> -shift;
		smon.loadavg[1] = la.ldavg[1] >> -shift;
		smon.loadavg[2] = la.ldavg[2] >> -shift;
	}
}
