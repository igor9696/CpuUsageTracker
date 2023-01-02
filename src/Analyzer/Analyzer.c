#include "Analyzer.h"
#include "../Reader/Reader.h"


cpuLoad_s CalculateCoreLoad(const cpuTimes_s* currentTimes, const cpuTimes_s* previousTimes, uint8_t core)
{
    cpuLoad_s ret = { 0 };
    uint64_t Idle = 0, NonIdle = 0, Total = 0, totald = 0, idled = 0;
    uint64_t PrevIdle = 0, PrevNonIdle = 0, PrevTotal = 0;
    uint8_t CPU_Percentage = 0;

    PrevIdle = previousTimes[core].idle + previousTimes[core].iowait;
    Idle = currentTimes[core].idle + currentTimes[core].iowait;

    PrevNonIdle = previousTimes[core].user + previousTimes[core].nice + previousTimes[core].system
                + previousTimes[core].irq + previousTimes[core].softirq + previousTimes[core].steal;

    NonIdle = currentTimes[core].user + currentTimes[core].nice + currentTimes[core].system  
            + currentTimes[core].irq + currentTimes[core].softirq + currentTimes[core].steal;

    PrevTotal = PrevIdle + PrevNonIdle;
    Total = Idle + NonIdle;

    totald = Total - PrevTotal;
    idled = Idle - PrevIdle;

    CPU_Percentage = ((float)(totald - idled) / totald) * 100;
    
    ret.core = core;
    ret.coreLoadPercentage = CPU_Percentage;

    return ret;
}
