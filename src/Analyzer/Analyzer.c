#include <string.h>
#include <stdlib.h>

#include "Analyzer.h"
#include "../Reader/Reader.h"
#include "../Logger/Logger.h"
#include "../Queue/Queue.h"

extern uint8_t systemNumberOfCores;
extern QueueHandle_t* cpuPercentageQueue;

static void UpdatePreviousCpuTimeData(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData)
{
    memcpy(*previousTimesData, *currentTimesData, (sizeof(cpuTimes_s) * systemNumberOfCores));
}

int AllocateMemoryPools(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad)
{
    *currentTimesData = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    *previousTimesData = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    *OutputLoad = malloc(sizeof(cpuLoad_s) * systemNumberOfCores);

    if(*currentTimesData == NULL || *previousTimesData == NULL || *OutputLoad == NULL)
    {
        LogPrintToFile("FUNC:%s Msg: Error during memory pool allocation!\n", __FUNCTION__);        
        return -1;
    }

    memset(*currentTimesData, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(*previousTimesData, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(*OutputLoad, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));

    return 0;
}

void DeallocateMemoryPools(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad)
{
    free(*currentTimesData);
    free(*previousTimesData);
    free(*OutputLoad);
}

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

void GetLoadFromEveryCore(cpuTimes_s** currentTimesData, cpuTimes_s** previousTimesData, cpuLoad_s** OutputLoad)
{
    for(int core_index = 0; core_index < systemNumberOfCores; core_index++)
    {
        (*OutputLoad)[core_index] = CalculateCoreLoad(*currentTimesData, *previousTimesData, core_index);
    }
    
    UpdatePreviousCpuTimeData(currentTimesData, previousTimesData);
}

void PushLoadDataToPrinter(const cpuLoad_s* OutputLoad)
{
    if (QueueSend(&cpuPercentageQueue, (const void*)OutputLoad) == 1)
    {
        LogPrintToFile("FUNC:%s Msg: cpuPercentageQueue is full! Element push skipped\n", __FUNCTION__);
    }
}

void PushLoadDataToLogger(const cpuLoad_s* OutputLoad)
{
    for(int core_idx = 0; core_idx < systemNumberOfCores; core_idx++)
    {
        LogPrintToFile("FUNC:%s Msg: Core: %2u, Load: %3u%%\n", 
                    __FUNCTION__, 
                    OutputLoad[core_idx].core,
                    OutputLoad[core_idx].coreLoadPercentage);
    }
}
