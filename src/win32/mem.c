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
#include <windows.h>
#include "xmon.h"

struct memstatex {
	unsigned __int32 length;
	unsigned __int32 mem_load;
	unsigned __int64 total_phys, avail_phys;
	unsigned __int64 total_pgfile, avail_pgfile;
	unsigned __int64 total_virt, avail_virt, avail_ext_virt;
};

typedef BOOL (WINAPI *globmemstatex_func)(struct memstatex*);
static globmemstatex_func gmemstatex;

int mem_init(void)
{
	/* try to find the modern GlobalMemoryStatusEx variant */
	HINSTANCE k32dll = GetModuleHandle("kernel32.dll");
	if(k32dll) {
		gmemstatex = (globmemstatex_func)GetProcAddress(k32dll, "GlobalMemoryStatusEx");
	}

	mem_update();
	return 0;
}

void mem_update(void)
{
	if(gmemstatex) {
		struct memstatex msx;
		msx.length = sizeof msx;
		gmemstatex(&msx);
		smon.mem_total = (unsigned long)(msx.total_phys >> 10);
		smon.mem_free = (unsigned long)(msx.avail_phys >> 10);
	} else {
		MEMORYSTATUS ms;
		GlobalMemoryStatus(&ms);
		smon.mem_total = ms.dwTotalPhys >> 10;
		smon.mem_free = ms.dwAvailPhys >> 10;
	}
}
