#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "Reader/Reader.h"
#include "Analyzer/Analyzer.h"
#include "Queue/Queue.h"

#define DEBUG 1
#define QUEUE_SIZE 10

uint8_t systemNumberOfCores;
pthread_mutex_t mutexCpuTime;
QueueHandle_t* cpuTimesQueue;
QueueHandle_t* cpuPercentageQueue;

void* Reader(void* arg)
{
    FILE *file = NULL;
    cpuTimes_s *cpuTimesArr = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    memset(cpuTimesArr, 0, sizeof(cpuTimes_s) * systemNumberOfCores);

    if(cpuTimesArr == NULL)
    {
        printf("Error during cpuTimesArr malloc!\n");
        return NULL;
    }

    for(;;)
    {
        char* line_buff = NULL;
        size_t line_len = 0;
        cpuTimes_s temp = { 0 };
        
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
                    cpuTimesArr[core_idx] = temp;
                    // printf("ReaderPROC: cpu=%u user=%llu nice=%llu system=%llu idle=%llu "
                    //     "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
                    //     cpu, cpuTimesArr[core_idx].user, cpuTimesArr[core_idx].nice, cpuTimesArr[core_idx].system, cpuTimesArr[core_idx].idle,
                    //     cpuTimesArr[core_idx].iowait, cpuTimesArr[core_idx].irq, cpuTimesArr[core_idx].softirq, cpuTimesArr[core_idx].steal, cpuTimesArr[core_idx].guest);
                    break;
                }
            }
        }

        if(fclose(file) != 0)
        {
            printf("Error during fclose!\n");
            break;
        }
        
        if(QueueSend(&cpuTimesQueue, (const void*)cpuTimesArr) == 1)
        {
            printf("Queue is full! Skipping adding new data\n");
        }
        
        free(line_buff);
        sleep(1);
    }

    free(cpuTimesArr);
    return arg;
}

void* Analyzer(void* arg)
{
    cpuTimes_s *currentTimesData = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    cpuTimes_s *previousTimesData = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    cpuLoad_s *CoreLoad = malloc(sizeof(cpuLoad_s) * systemNumberOfCores);

    memset(currentTimesData, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(previousTimesData, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(CoreLoad, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));

    if(currentTimesData == NULL || previousTimesData == NULL || CoreLoad == NULL)
    {
        printf("Error during analyzer malloc!\n");
        return NULL;
    }

    for(;;)
    {   
        QueueBlockingReceive(&cpuTimesQueue, (void*)currentTimesData);

        for(int core_index = 0; core_index < systemNumberOfCores; core_index++)
        {
            CoreLoad[core_index] = CalculateCoreLoad(currentTimesData, previousTimesData, core_index);
        }

        /* Update previous cpuTimes and push CoreLoad to queue*/
        memcpy((void*)previousTimesData, (const void*)currentTimesData, (sizeof(cpuTimes_s) * systemNumberOfCores));
        if (QueueSend(&cpuPercentageQueue, (const void*)CoreLoad) == 1)
        {
            printf("cpuPercentageQueue is full! Skipping...\n");
        }
    }

    free(currentTimesData);
    free(previousTimesData);
    free(CoreLoad);

    return arg;
}

void* Printer(void* arg)
{
    cpuLoad_s CoreLoad[systemNumberOfCores];
    memset(&CoreLoad, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));

    for(;;)
    {
        // TODO: implement QueueReceive
        QueueBlockingReceive(&cpuPercentageQueue, (void*)&CoreLoad);

        printf("\rCPU core usage:");
        for(int core = 0; core < systemNumberOfCores; core++)
        {
           printf("\tCORE %2u: %3u%%", CoreLoad[core].core, CoreLoad[core].coreLoadPercentage);
        }
        fflush(stdout);
        
        sleep(1);
    }
    return arg;
}


int main()
{
    systemNumberOfCores = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t th[5];
    pthread_mutex_init(&mutexCpuTime, NULL);
    cpuTimesQueue = CreateQueue(QUEUE_SIZE, (sizeof(cpuTimes_s) * systemNumberOfCores));
    cpuPercentageQueue = CreateQueue(QUEUE_SIZE, (sizeof(cpuLoad_s) * systemNumberOfCores));

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

        else if(t == 2)
        {
            if(pthread_create(&th[t], NULL, Printer, NULL) != 0)
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
    DestroyQueue(&cpuPercentageQueue);
    return 0;

CleanUp:
    return 1;
}
