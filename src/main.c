#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Reader/Reader.h"
#include "Queue/Queue.h"

// #define DEBUG 1

pthread_mutex_t mutexCpuTime;
QueueHandle_t* cpuTimesQueue;


void* Reader(void* arg)
{
    FILE *file = NULL;
    cpuTimes_s cpuTimesArr[MAX_NUMBER_OF_CORES] = {0};
    
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

        if(fclose(file) != 0)
        {
            printf("Error during fclose!\n");
            break;
        }
        
        if(QueueSend(&cpuTimesQueue, (const void*)&cpuTimesArr) == 1)
        {
            printf("Queue is full! Skipping adding new data\n");
        }

        free(line_buff);
        sleep(1);
    }

    return arg;
}

void* Analyzer(void* arg)
{
    cpuTimes_s input_raw_data[MAX_NUMBER_OF_CORES] = { 0 };
    for(;;)
    {   
        QueueBlockingReceive(&cpuTimesQueue, (void*)&input_raw_data);
        for(int c = 0; c < MAX_NUMBER_OF_CORES; c++)
        {
            printf("[A]PROC: cpu=%u user=%llu nice=%llu system=%llu idle=%llu "
                "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
                c, input_raw_data[c].user, input_raw_data[c].nice, input_raw_data[c].system, input_raw_data[c].idle,
                input_raw_data[c].iowait, input_raw_data[c].irq, input_raw_data[c].softirq, input_raw_data[c].steal, input_raw_data[c].guest);
        }







    }




    return arg;
}



int main()
{
    pthread_t th[5];
    pthread_mutex_init(&mutexCpuTime, NULL);
    cpuTimesQueue = CreateQueue(10, (sizeof(cpuTimes_s) * MAX_NUMBER_OF_CORES));

    for(int t = 0; t < 5; t++)
    {
        if(t == 0)
        {
            if(pthread_create(&th[t], NULL, Reader, NULL) != 0)
            {
                printf("Pthread_create error!\n");
                goto CleanUp;
            }
        }

        else if(t == 1)
        {
            if(pthread_create(&th[t], NULL, Analyzer, NULL) != 0)
            {
                printf("Pthread_create error!\n");
                goto CleanUp;
            }
        }

    }

    for(int t = 0; t < 5; t++)
    {
        if(pthread_join(th[t], NULL) != 0)
        {
            printf("Pthread_create error!\n");
            return -1;
        }
    }

    pthread_mutex_destroy(&mutexCpuTime);
    DestroyQueue(&cpuTimesQueue);
    return 0;

CleanUp:
    return 1;
}
