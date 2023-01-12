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
#include "Watchdog/Watchdog.h"

#define DEBUG 1
#define QUEUE_SIZE 10
#define SIG_NOT_ACTIVE 0
#define SIG_ACTIVE 1
#define NUMBER_OF_THREADS 5

/* Global variable declarations */
uint8_t systemNumberOfCores;
QueueHandle_t* cpuTimesQueue;
QueueHandle_t* cpuPercentageQueue;
volatile sig_atomic_t SIG_FLAG = 0;
uint8_t THREADS_TERMINATED_FLAG = 0;

/* Function declarations */
void CleanUp_Handler(int signum);
void CreateThreads(pthread_t* th);
int JoinThreads(pthread_t* th);
void SigHandlerInit(struct sigaction* action);

void* Reader(void* arg)
{
    WatchdogRegister();
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
        WatchdogUpdate();
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

    WatchdogRegister();

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
        PushLoadDataToLogger(CoreLoad);
        WatchdogUpdate();
    }

    DeallocateMemoryPools(&currentTimesData, &previousTimesData, &CoreLoad);
    LogPrintToFile("FUNC:%s Msg: Analyzer thread closed\n", __FUNCTION__);
    return arg;
}

void* Printer(void* arg)
{
    cpuLoad_s CoreLoad[systemNumberOfCores];
    memset(&CoreLoad, 0, (sizeof(cpuLoad_s) * systemNumberOfCores));
    WatchdogRegister();

    while(SIG_FLAG == SIG_NOT_ACTIVE)
    {
        if(0 == QueueNonBlockingReceive(&cpuPercentageQueue, (void*)&CoreLoad))
        {
            PrintFormattedCoreUsage(CoreLoad, systemNumberOfCores);
        }
        WatchdogUpdate();
        sleep(1);
    }

    LogPrintToFile("FUNC:%s Msg: Printer thread closed\n", __FUNCTION__);
    return arg;
}

void* Watchdog(void* arg)
{
    WatchdogRegister();
    while(SIG_FLAG == SIG_NOT_ACTIVE)
    {        
        sleep(2);
        WatchdogUpdate();
        if(-1 == WatchdogCheck())
        {
            printf("Watchdog tirggered!\n");
            LogPrintToFile("FUNC:%s Watchdog timeout!\n", __FUNCTION__);
            sleep(1);
            exit(EXIT_FAILURE);
        }
        else
        {
            LogPrintToFile("FUNC:%s Msg: Watchdog OK\n", __FUNCTION__);
        }
    }

    LogPrintToFile("FUNC:%s MSG: Watchod thread closed\n", __FUNCTION__);
    return arg;
}

void* Logger(void* arg)
{
    WatchdogRegister();
    while(THREADS_TERMINATED_FLAG == 0)
    {
        ProcessLogDataToFile();
        WatchdogUpdate();
    }

    LogPrintToFile("FUNC:%s Msg: Logger thread closed\n", __FUNCTION__);
    return arg;
}

int main()
{
    pthread_t ThreadPool[NUMBER_OF_THREADS];
    struct sigaction action;
    systemNumberOfCores = sysconf(_SC_NPROCESSORS_ONLN);
    Logger_Init();
    WatchdogInit(NUMBER_OF_THREADS);
    SigHandlerInit(&action);

    cpuTimesQueue       = CreateQueue(QUEUE_SIZE, (sizeof(cpuTimes_s) * systemNumberOfCores));
    cpuPercentageQueue  = CreateQueue(QUEUE_SIZE, (sizeof(cpuLoad_s) * systemNumberOfCores));

    CreateThreads(ThreadPool);
    LogPrintToFile("FUNC:%s Msg: CUT application started\n", __FUNCTION__);
    JoinThreads(ThreadPool);

    /* Clean resources */
    DestroyQueue(&cpuTimesQueue);
    DestroyQueue(&cpuPercentageQueue);
    Logger_DeInit();
    WatchdogDeinit();

    printf("CUT app closed. \n");
    LogPrintToFile("FUNC:%s MSG: CUT app closed!\n", __FUNCTION__);
    return 0;
}

void CleanUp_Handler(int signum)
{
    printf("\nCleaning resources...\n");
    LogPrintToFile("FUNC:%s Msg: Cleaning resources\n", __FUNCTION__);
    SIG_FLAG = signum;
    sem_post(&cpuTimesQueue->_QueueCntSem);
}

void CreateThreads(pthread_t* th)
{
    for(int t = 0; t < NUMBER_OF_THREADS; t++)
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
}

int JoinThreads(pthread_t* th)
{
    for(int t = 0; t < NUMBER_OF_THREADS; t++)
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

    return 0;
}

void SigHandlerInit(struct sigaction* action)
{
    memset(action, 0, sizeof(struct sigaction));
    action->sa_handler = CleanUp_Handler;
    sigaction(SIGTERM, action, NULL);
    sigaction(SIGINT, action, NULL);
}