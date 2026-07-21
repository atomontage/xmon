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
#include <unistd.h>
#include <sys/sysmp.h>
#include "xmon.h"

static int pageshift;

int mem_init(void)
{
	long pagesz;

	pagesz = sysconf(_SC_PAGESIZE);

	pageshift = 0;
	while(pagesz > 1) {
		pagesz >>= 1;
		pageshift++;
	}

	return 0;
}

void mem_update(void)
{
	struct rminfo rm;

	if(sysmp(MP_SAGET, MPSA_RMINFO, &rm, sizeof rm) == -1) {
		fprintf(stderr, "sysmp failed\n");
	}
	smon.mem_total = rm.physmem << (pageshift - 10);
	smon.mem_free = rm.freemem << (pageshift - 10);
}
