#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Reader.h"
#include "Logger.h"
#include "Queue.h"

extern uint8_t systemNumberOfCores;
extern QueueHandle_t* cpuTimesQueue;

cpuTimes_s* GetCpuTimeMemoryPool()
{
    cpuTimes_s *cpuTimesArr = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    if(cpuTimesArr == NULL)
    {
        return NULL;
    }

    memset(cpuTimesArr, 0, sizeof(cpuTimes_s) * systemNumberOfCores);
    return cpuTimesArr;
}

void DeallocateCpuTimeMemoryPool(cpuTimes_s** cpuTimes)
{
    free(*cpuTimes);
    *cpuTimes = NULL;
}

void GetProcStatRaw(cpuTimes_s** cpuTimesRawOutput)
{
    FILE *file = NULL;
    char *line_buff = NULL;
    size_t line_len = 0;
    cpuTimes_s temp = { 0 };

    if((file = fopen("/proc/stat", "r")) == NULL)
    {
        LogPrintToFile("FUNC:%s Msg: Error during file open!\n", __FUNCTION__);
        return;
    }

    while(getline(&line_buff, &line_len, file) != -1)
    {
        for(int core_idx = 0; core_idx < MAX_NUMBER_OF_CORES; core_idx++)
        {
            int ret = 0;
            int cpu = -1;

            ret = sscanf(line_buff, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu",
            &cpu,
            &temp.user,
            &temp.nice,
            &temp.system,
            &temp.idle,
            &temp.iowait,
            &temp.irq,
            &temp.softirq,
            &temp.steal,
            &temp.guest);

            if ((cpu == core_idx) && (ret == 10))
            {
                (*cpuTimesRawOutput)[core_idx] = temp;
                break;
            }
        }
    }

    if(fclose(file) != 0)
    {
        LogPrintToFile("FUNC:%s Msg: Error during fclose!\n", __FUNCTION__);
    }

    free(line_buff);
}

void PushRawCpuTimesToLogger(const cpuTimes_s* cpuTimesRaw)
{
    for(int core_idx = 0; core_idx < systemNumberOfCores; core_idx++)
    {
        LogPrintToFile("FUNC:%s Msg: cpu=%d user=%llu nice=%llu system=%llu idle=%llu "
                    "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
                    __FUNCTION__,
                    core_idx,
                    cpuTimesRaw[core_idx].user, 
                    cpuTimesRaw[core_idx].nice, 
                    cpuTimesRaw[core_idx].system, 
                    cpuTimesRaw[core_idx].idle,
                    cpuTimesRaw[core_idx].iowait, 
                    cpuTimesRaw[core_idx].irq, 
                    cpuTimesRaw[core_idx].softirq, 
                    cpuTimesRaw[core_idx].steal,
                    cpuTimesRaw[core_idx].guest
        );
    }
}

void PushRawCpuTimesToAnalyzer(const cpuTimes_s* cpuTimesRaw)
{
    if(QueueSend(&cpuTimesQueue, (const void*)cpuTimesRaw) == 1)
    {
        LogPrintToFile("FUNC:%s Msg: cpuTimeQueue is full!\n", __FUNCTION__);
    }
}