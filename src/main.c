#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Reader/Reader.h"

#define DEBUG 1

cpuTimes_s cpuTimesArr[MAX_NUMBER_OF_CORES] = {0};
pthread_mutex_t mutexCpuTime;

void* Reader(void* arg)
{
    FILE *file = NULL;
    
    for(;;)
    {
        char* line_buff = NULL;
        size_t line_len = 0;

        /*Get raw data from /proc/stat */
        if((file = fopen("/proc/stat", "r")) == NULL)
        {
            printf("Error during file open!\n");
            break;
        }

        pthread_mutex_lock(&mutexCpuTime);
        while(getline(&line_buff, &line_len, file) != -1)
        {
            for(int core_idx = 0; core_idx < MAX_NUMBER_OF_CORES; core_idx++)
            {
                int ret = 0;
                int cpu = 0;

                ret = sscanf(line_buff, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &cpu,
                &cpuTimesArr[core_idx].user,
                &cpuTimesArr[core_idx].nice,
                &cpuTimesArr[core_idx].system,
                &cpuTimesArr[core_idx].idle,
                &cpuTimesArr[core_idx].iowait,
                &cpuTimesArr[core_idx].irq,
                &cpuTimesArr[core_idx].softirq,
                &cpuTimesArr[core_idx].steal,
                &cpuTimesArr[core_idx].guest);

                #ifdef DEBUG
                if ((cpu == core_idx) && (ret == 10))
                {
                    printf("PROC: cpu=%u user=%llu nice=%llu system=%llu idle=%llu "
                        "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
                        cpu, cpuTimesArr[core_idx].user, cpuTimesArr[core_idx].nice, cpuTimesArr[core_idx].system, cpuTimesArr[core_idx].idle,
                        cpuTimesArr[core_idx].iowait, cpuTimesArr[core_idx].irq, cpuTimesArr[core_idx].softirq, cpuTimesArr[core_idx].steal, cpuTimesArr[core_idx].guest);
                }
                #endif
            }
        }
        pthread_mutex_unlock(&mutexCpuTime);

        if(fclose(file) != 0)
        {
            printf("Error during fclose!\n");
            break;
        }
        
        free(line_buff);
        sleep(1);
    }

    return arg;
}


int main()
{
    pthread_t th[5];
    pthread_mutex_init(&mutexCpuTime, NULL);

    // Reader Task
    if(pthread_create(&th[0], NULL, Reader, NULL) != 0)
    {
        printf("Pthread_create error!\n");
        goto CleanUp;
    }

    if(pthread_join(th[0], NULL) != 0)
    {
        printf("Pthread_create error!\n");
        return -1;
    }

    pthread_mutex_destroy(&mutexCpuTime);
    printf("Cpu Usage Tracker \n");
    return 0;

CleanUp:
    return -1;
}
