// plat/raspi/cpu_load.c
#include "raspi/cpu_load.h"
#include <uk/plat/common/lcpu.h>

uint64_t      cntfrq;
uint64_t      last_ccnt[CONFIG_UKPLAT_LCPU_MAXCOUNT] = {0};

void cpu_load_init(void)
{
    int c = ukplat_lcpu_id();

    asm volatile("mrs %0, CNTFRQ_EL0"  : "=r"(cntfrq));
    asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(last_ccnt[c]));
}

float cpu_load_sample(void)
{
    int c = ukplat_lcpu_id();

    // Read the current value of the cycle counter (PMCCNTR_EL0)
    // This register increments once per CPU cycle.
    uint64_t cc;
    asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(cc));

    // Compute how many cycles have elapsed since the last sample
    // last_ccnt[c] was set to the previous PMCCNTR value on this core.
    uint64_t delta = cc - last_ccnt[c];

    // Save the current counter as the “last” for the next call
    last_ccnt[c] = cc;

    // Divide by the CPU’s cycle‐per‐second rate (cntfrq) to get
    // the fraction of one second that the core was “busy” executing.
    //
    // e.g. if delta==cpu_freq, load==1.0 → 100% busy
    //      if delta==cpu_freq/2, load==0.5 → 50% busy
    float load = (float)delta / (float)cntfrq;

    return load;
}
