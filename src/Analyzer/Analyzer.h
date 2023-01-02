#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdint.h>
#include "../Reader/Reader.h"

typedef struct cpuLoad_s
{
    uint8_t core;
    uint8_t coreLoadPercentage;
} cpuLoad_s;


cpuLoad_s CalculateCoreLoad(const cpuTimes_s* currentTimes, const cpuTimes_s* previousTimes,
                        uint8_t core_number);

#endif // ANALYZER_H
