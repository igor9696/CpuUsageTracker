#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#include "Watchdog.h"
#include "../Logger/Logger.h"

#define _GNU_SOURCE

typedef struct wdgthread_t
{
    int _internalCnt;
    int _internalPrevCnt;
    pid_t _threadID;
} wdgthread_t;

typedef struct watchdog_t
{
    int _registeredThreads;
    int _head;
    int _tail;
    wdgthread_t* ThreadsArr;
} watchdog_t;

static pthread_mutex_t watchdogMutex;
static pthread_mutex_t watchdogUpdateCheckMutex;

static watchdog_t Watchdog = { 0 };


int WatchdogInit(const int NumOfThreads)
{
    Watchdog._head = 0;
    Watchdog._tail = 0;
    Watchdog._registeredThreads = 0;
    Watchdog.ThreadsArr = malloc(sizeof(wdgthread_t) * NumOfThreads);
    if(Watchdog.ThreadsArr == NULL)
    {
        return -1;
    }
    memset(Watchdog.ThreadsArr, 0, (sizeof(wdgthread_t) * NumOfThreads));
    pthread_mutex_init(&watchdogMutex, NULL);
    pthread_mutex_init(&watchdogUpdateCheckMutex, NULL);
    return 0;
}

void WatchdogDeinit()
{
    pthread_mutex_destroy(&watchdogMutex);
    pthread_mutex_destroy(&watchdogUpdateCheckMutex);
    free(Watchdog.ThreadsArr);
    Watchdog.ThreadsArr = NULL;
}

void WatchdogRegister()
{
    pid_t callerThreadId = syscall(SYS_gettid);
    wdgthread_t temp = { 0 };
    temp._internalCnt = 0;
    temp._threadID = callerThreadId;
    temp._internalPrevCnt = 0;

    pthread_mutex_lock(&watchdogMutex);
    Watchdog._registeredThreads++;
    Watchdog.ThreadsArr[Watchdog._head] = temp;
    Watchdog._head++;
    pthread_mutex_unlock(&watchdogMutex);
}

void WatchdogUpdate()
{  
    pid_t callerThreadId = syscall(SYS_gettid);
    for(int idx = 0; idx < Watchdog._registeredThreads; idx++)
    {
        if(Watchdog.ThreadsArr[idx]._threadID == callerThreadId)
        {
            pthread_mutex_lock(&watchdogUpdateCheckMutex);
            Watchdog.ThreadsArr[idx]._internalCnt++;   
            pthread_mutex_unlock(&watchdogUpdateCheckMutex);
            break;
        }
    }
}

int WatchdogCheck()
{   
    pthread_mutex_lock(&watchdogUpdateCheckMutex);
    for(int idx = 0; idx < Watchdog._registeredThreads; idx++)
    {
        if(Watchdog.ThreadsArr[idx]._internalCnt == Watchdog.ThreadsArr[idx]._internalPrevCnt)
        {
            LogPrintToFile("FUNC:%s Msg: Watchdog triggered! Thread: %d not responsive!\n",
                        __FUNCTION__,
                        Watchdog.ThreadsArr[idx]._threadID);
            return -1;
        }
        else
        {
            Watchdog.ThreadsArr[idx]._internalPrevCnt = Watchdog.ThreadsArr[idx]._internalCnt;
        }
    }
    pthread_mutex_unlock(&watchdogUpdateCheckMutex);
    return 0;
}