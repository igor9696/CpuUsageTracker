#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdint.h>
#include "../Reader/Reader.h"

typedef struct cpuLoad_s
{
    uint8_t core;
    uint8_t coreLoadPercentage;
} cpuLoad_s;

int AllocateMemoryPools(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad);
void DeallocateMemoryPools(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad);
cpuLoad_s CalculateCoreLoad(const cpuTimes_s* currentTimes, const cpuTimes_s* previousTimes, uint8_t core_number);
void GetLoadFromEveryCore(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad);
void PushLoadDataToPrinter(const cpuLoad_s* OutputLoad);
void PushLoadDataToLogger(const cpuLoad_s* OutputLoad);


#endif // ANALYZER_H
