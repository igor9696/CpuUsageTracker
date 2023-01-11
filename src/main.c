#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "Reader/Reader.h"
#include "Analyzer/Analyzer.h"
#include "Queue/Queue.h"
#include "Printer/Printer.h"
#include "Logger/Logger.h"

#define DEBUG 1
#define QUEUE_SIZE 10
#define SIG_NOT_ACTIVE 0
#define SIG_ACTIVE 1

uint8_t READER_WD_FLAG = 1;
uint8_t ANALYZER_WD_FLAG = 1;
uint8_t PRINTER_WD_FLAG = 1;

/* Global variable declarations */
uint8_t systemNumberOfCores;
pthread_mutex_t mutexCpuTime;
QueueHandle_t* cpuTimesQueue;
QueueHandle_t* cpuPercentageQueue;
volatile sig_atomic_t SIG_FLAG = 0;
uint8_t THREADS_TERMINATED_FLAG = 0;

/* Function declarations */
void CleanUp_Handler(int signum);

void* Reader(void* arg)
{
    cpuTimes_s *cpuTimesRaw = GetCpuTimeMemoryPool();
    if(cpuTimesRaw == NULL)
    {
        LogPrintToFile("FUNC:%s Msg: Error during MemoryPool allocation!\n", __FUNCTION__);
        return arg;
    }

    while(SIG_FLAG == SIG_NOT_ACTIVE)
    {
        GetProcStatRaw(&cpuTimesRaw);
        PushRawCpuTimesToLogger(cpuTimesRaw);
        PushRawCpuTimesToAnalyzer(cpuTimesRaw);

        READER_WD_FLAG = 1;
        // UpdateWatchdog();
        sleep(1);
    }
    
    DeallocateCpuTimeMemoryPool(&cpuTimesRaw);
    LogPrintToFile("FUNC:%s Msg: Reader thread closed\n", __FUNCTION__);
    return arg;
}

void* Analyzer(void* arg)
{
    cpuTimes_s* currentTimesData;
    cpuTimes_s* previousTimesData;
    cpuLoad_s* CoreLoad;

    if(-1 == AllocateMemoryPools(&currentTimesData, &previousTimesData, &CoreLoad))
    {
        return NULL;
    }
    
    while(SIG_FLAG == SIG_NOT_ACTIVE)
    {   
        QueueBlockingReceive(&cpuTimesQueue, (void*)currentTimesData);
        if(SIG_FLAG != SIG_NOT_ACTIVE)
        {
            break;
        }
        GetLoadFromEveryCore(&currentTimesData, &previousTimesData, &CoreLoad);
        PushLoadDataToPrinter(CoreLoad);
        ANALYZER_WD_FLAG = 1;    
    }

    DeallocateMemoryPools(&currentTimesData, &previousTimesData, &CoreLoad);
    LogPrintToFile("FUNC:%s Msg: Analyzer thread closed\n", __FUNCTION__);
    return arg;
}

void* Printer(void* arg)
{
    cpuLoad_s CoreLoad[systemNumberOfCores];
    memset(&CoreLoad, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));
    int ret = -1;

    while(SIG_FLAG == 0)
    {
        ret = QueueNonBlockingReceive(&cpuPercentageQueue, (void*)&CoreLoad);
        if(ret == 0)
        {
            PrintFormattedCoreUsage(CoreLoad, systemNumberOfCores);
        }

        PRINTER_WD_FLAG = 1;
        sleep(1);
    }

    LogPrintToFile("FUNC:%s MSG: Printer thread closed\n", __FUNCTION__);
    return arg;
}

void* Watchdog(void* arg)
{
    /* If threads won't set WD_FLAG within 2 sec indicating that they are working, close program */
    while(SIG_FLAG == 0)
    {
        if((READER_WD_FLAG && ANALYZER_WD_FLAG && PRINTER_WD_FLAG) == 0)
        {
            LogPrintToFile("FUNC:%s Watchdog timeout!\n", __FUNCTION__);
            exit(EXIT_FAILURE);
            break;
        }

        READER_WD_FLAG = 0;
        ANALYZER_WD_FLAG = 0;
        PRINTER_WD_FLAG = 0;
        LogPrintToFile("FUNC:%s MSG: Watchdog reseted\n", __FUNCTION__);
        sleep(2);
    }

    LogPrintToFile("FUNC:%s MSG: Watchod thread closed\n", __FUNCTION__);
    return arg;
}

void* Logger(void* arg)
{
    while(THREADS_TERMINATED_FLAG == 0)
    {
        ProcessLogDataToFile();
    }

    LogPrintToFile("FUNC:%s MSG: Logger thread closed\n", __FUNCTION__);
    return arg;
}


int main()
{
    pthread_t th[5];
    systemNumberOfCores = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_mutex_init(&mutexCpuTime, NULL);
    Logger_Init();

    cpuTimesQueue       = CreateQueue(QUEUE_SIZE, (sizeof(cpuTimes_s) * systemNumberOfCores));
    cpuPercentageQueue  = CreateQueue(QUEUE_SIZE, (sizeof(cpuLoad_s) * systemNumberOfCores));

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = CleanUp_Handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    for(int t = 0; t < 5; t++)
    {
        if(t == 0)
        {
            if(pthread_create(&th[t], NULL, Reader, NULL) != 0)
            {
                printf("Pthread_create error!\n");
            }
        }

        else if(t == 1)
        {
            if(pthread_create(&th[t], NULL, Analyzer, NULL) != 0)
            {
                printf("Pthread_create error!\n");
            }
        }

        else if(t == 2)
        {
            if(pthread_create(&th[t], NULL, Printer, NULL) != 0)
            {
                printf("Pthread_create error!\n");
            }
        }

        else if(t == 3)
        {
            if(pthread_create(&th[t], NULL, Watchdog, NULL) != 0)
            {
                printf("Pthread_create error!\n");
            }
        }

        else if(t == 4)
        {
            if(pthread_create(&th[t], NULL, Logger, NULL) != 0)
            {
                printf("Pthread_create error!\n");
            }
        }

    }

    LogPrintToFile("FUNC:%s Msg: CUT application started\n", __FUNCTION__);

    for(int t = 0; t < 5; t++)
    {
        if(t == 4)
        {
            THREADS_TERMINATED_FLAG = 1;
            if(pthread_join(th[t], NULL) != 0)
            {
                printf("Pthread_join error!\n");
            }
        }

        else if(pthread_join(th[t], NULL) != 0)
        {
            printf("Pthread_join error!\n");
            return -1;
        }
    }

    pthread_mutex_destroy(&mutexCpuTime);
    DestroyQueue(&cpuTimesQueue);
    DestroyQueue(&cpuPercentageQueue);
    Logger_DeInit();

    printf("CUT app closed. \n");
    LogPrintToFile("FUNC:%s MSG: CUT app closed!\n", __FUNCTION__);
    return 0;
}


void CleanUp_Handler(int signum)
{
    printf("\nCleaning resources...\n");
    LogPrintToFile("FUNC:%s MSG: Cleaning resources\n", __FUNCTION__);
    SIG_FLAG = signum;
    sem_post(&cpuTimesQueue->_QueueCntSem);
}