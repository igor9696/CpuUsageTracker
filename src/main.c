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
    uint64_t Idle = 0, NonIdle = 0, Total = 0, totald = 0, idled = 0;
    uint64_t PrevIdle = 0, PrevNonIdle = 0, PrevTotal = 0;
    float CPU_Percentage = 0;

    cpuTimes_s *input_raw_data = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    cpuTimes_s *prev_raw_data = malloc(sizeof(cpuTimes_s) * systemNumberOfCores);
    cpuLoad_s *load = malloc(sizeof(cpuLoad_s) * systemNumberOfCores);

    memset(input_raw_data, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(prev_raw_data, 0, (sizeof(cpuTimes_s) * systemNumberOfCores));
    memset(load, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));

    if(input_raw_data == NULL || prev_raw_data == NULL || load == NULL)
    {
        printf("Error during analyzer malloc!\n");
        return NULL;
    }

    for(;;)
    {   
        QueueBlockingReceive(&cpuTimesQueue, (void*)input_raw_data);

        for(int c = 0; c < systemNumberOfCores; c++)
        {
            PrevIdle = prev_raw_data[c].idle + prev_raw_data[c].iowait;
            Idle = input_raw_data[c].idle + input_raw_data[c].iowait;

            PrevNonIdle = prev_raw_data[c].user + prev_raw_data[c].nice + prev_raw_data[c].system
                        + prev_raw_data[c].irq + prev_raw_data[c].softirq + prev_raw_data[c].steal;

            NonIdle = input_raw_data[c].user + input_raw_data[c].nice + input_raw_data[c].system  
                    + input_raw_data[c].irq + input_raw_data[c].softirq + input_raw_data[c].steal;

            PrevTotal = PrevIdle + PrevNonIdle;
            Total = Idle + NonIdle;

            totald = Total - PrevTotal;
            idled = Idle - PrevIdle;

            CPU_Percentage = ((float)(totald - idled) / totald) * 100;
            load[c].core = c;
            load[c].coreLoadPercentage = (uint8_t)CPU_Percentage;

            #ifdef DEBUG
            // printf("[A]PROC: cpu=%u user=%llu nice=%llu system=%llu idle=%llu "
            //     "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
            //     c, input_raw_data[c].user, input_raw_data[c].nice, input_raw_data[c].system, input_raw_data[c].idle,
            //     input_raw_data[c].iowait, input_raw_data[c].irq, input_raw_data[c].softirq, input_raw_data[c].steal, input_raw_data[c].guest);
            printf("CPU Percentage of core %d is %u\n", load[c].core, load[c].coreLoadPercentage);
            #endif
        }

        memcpy((void*)prev_raw_data, (const void*)input_raw_data, (sizeof(cpuTimes_s) * systemNumberOfCores));

        if (QueueSend(&cpuPercentageQueue, (const void*)&load) == 1)
        {
            printf("cpuPercentageQueue is full! Skipping...\n");
        }
    }

    free(input_raw_data);
    free(prev_raw_data);
    free(load);

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
