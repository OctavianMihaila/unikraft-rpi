// plat/raspi/cpu_load.h
#pragma once
#include <stdint.h>

/// How many samples/phases you’ll take per core
#define CPU_LOAD_SAMPLES  3

/// Must be called once on each core before your first sample
void cpu_load_init(void);

/// Snapshot this core’s CCNT, compute load% since last call, return it
float cpu_load_sample(void);

