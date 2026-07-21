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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>

#include "xmon.h"

static int cpu_count;
static host_t host_port;
static processor_cpu_load_info_t prev_load;

const int MAX_CPU_USAGE = 128;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int cpu_init(void)
{
	natural_t pc;
	kern_return_t kr;
	mach_msg_type_number_t msg_count;
	processor_cpu_load_info_t info;

	size_t bytes;

	host_port = mach_host_self();
	kr = host_processor_info(host_port, PROCESSOR_CPU_LOAD_INFO, &pc,
				 (processor_info_array_t *)&info, &msg_count);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "host_processor_info failed\n");
		return -1;
	}

	cpu_count = pc;
	smon.num_cpus = cpu_count;
	if (!(smon.cpu = calloc(cpu_count, sizeof *smon.cpu))) {
		fprintf(stderr, "failed to allocate cpu buffer\n");
		return -1;
	}

	bytes = msg_count * sizeof(integer_t);
	if (!(prev_load = malloc(bytes))) {
		fprintf(stderr, "failed to allocate prev_load\n");
		return -1;
	}
	memcpy(prev_load, info, bytes);

	vm_deallocate(mach_task_self(), (vm_address_t)info, bytes);

	return 0;
}

void cpu_shutdown(void)
{
}

static int calc_usage(processor_cpu_load_info_t cur, processor_cpu_load_info_t prev)
{
	uint64_t total = 0, used = 0;
	uint64_t d_user, d_sys, d_nice, d_idle;

	d_user = cur->cpu_ticks[CPU_STATE_USER] - prev->cpu_ticks[CPU_STATE_USER];
	d_sys = cur->cpu_ticks[CPU_STATE_SYSTEM] - prev->cpu_ticks[CPU_STATE_SYSTEM];
	d_nice = cur->cpu_ticks[CPU_STATE_NICE] - prev->cpu_ticks[CPU_STATE_NICE];
	d_idle = cur->cpu_ticks[CPU_STATE_IDLE] - prev->cpu_ticks[CPU_STATE_IDLE];

	used = d_user + d_sys + d_nice;
	total = used + d_idle;

	if (total == 0) return 0;

	return (int)((used * MAX_CPU_USAGE) / total);
}

void cpu_update(void)
{
	natural_t pc;
	kern_return_t kr;
	mach_msg_type_number_t msg_count;
	processor_cpu_load_info_t info;

	int i;
	int total_val = 0;
	size_t bytes;

	kr = host_processor_info(host_port, PROCESSOR_CPU_LOAD_INFO, &pc,
				 (processor_info_array_t *)&info, &msg_count);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "host_processor_info failed\n");
		return;
	}

	/* The number of processors shouldn't change */
	assert(cpu_count == (int)pc);

	for (i = 0; i < cpu_count; i++) {
		smon.cpu[i] = calc_usage(&info[i], &prev_load[i]);
		smon.cpu[i] = MIN(smon.cpu[i], MAX_CPU_USAGE - 1);
		total_val += smon.cpu[i];
	}

	smon.single = MIN(total_val / cpu_count, MAX_CPU_USAGE - 1);

	bytes = msg_count * sizeof(integer_t);
	memcpy(prev_load, info, bytes);
	vm_deallocate(mach_task_self(), (vm_address_t)info, bytes);
}
