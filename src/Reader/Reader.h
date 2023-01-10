#ifndef READER_H
#define READER_H

#include <stdint.h>

#define MAX_NUMBER_OF_CORES 32

typedef unsigned long long cputime_t;

typedef struct cpuTimes_s
{
    cputime_t user;
    cputime_t nice;
    cputime_t system;
    cputime_t idle;
    cputime_t iowait;
    cputime_t irq;
    cputime_t softirq;
    cputime_t steal;
    cputime_t guest;
} cpuTimes_s;


cpuTimes_s* GetCpuTimeMemoryPool();
void DeallocateCpuTimeMemoryPool(cpuTimes_s** cpuTimes);
void GetProcStatRaw(cpuTimes_s** cpuTimesRawOutput);
void PushRawCpuTimesToLogger(const cpuTimes_s* cpuTimesRaw);
void PushRawCpuTimesToAnalyzer(const cpuTimes_s* cpuTimesRaw);


#endif // READER_H