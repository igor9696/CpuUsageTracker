#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>

#include "Logger.h"
#include "../Analyzer/Analyzer.h"
#include "../Reader/Reader.h"
#include "../Queue/Queue.h"
#include "string.h"

extern uint8_t systemNumberOfCores;
static pthread_mutex_t fileMutex;

int Logger_Init()
{
    return pthread_mutex_init(&fileMutex, NULL);
}

void Logger_DeInit()
{
    pthread_mutex_destroy(&fileMutex);
}

static void LogReaderThreadData(const loggerData_s* data)
{
    FILE* file = NULL;
    char buffer[MAX_MESSAGE_SIZE] = { 0 };
    cpuTimes_s* cpuTimes = NULL;
    cpuTimes = (cpuTimes_s*)data->message;

    pthread_mutex_lock(&fileMutex);
    file = fopen(FILE_NAME, "a");
    if(file == NULL)
    {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    for(int core = 0; core < systemNumberOfCores; core++)
    {
        size_t ret = 0;
        ret = snprintf(buffer, sizeof(buffer), "Time:%ld THREAD READER: cpu=%d user=%llu nice=%llu system=%llu idle=%llu "
                        "iowait=%llu irq=%llu softirq=%llu steal=%llu guest=%llu\n",
                        data->timestamp,
                        core,
                        cpuTimes[core].user, 
                        cpuTimes[core].nice, 
                        cpuTimes[core].system, 
                        cpuTimes[core].idle,
                        cpuTimes[core].iowait, 
                        cpuTimes[core].irq, 
                        cpuTimes[core].softirq, 
                        cpuTimes[core].steal,
                        cpuTimes[core].guest);
        fwrite(buffer, sizeof(char), ret, file);
        memset(buffer, 0, sizeof(buffer));
    }
    fclose(file);
    pthread_mutex_unlock(&fileMutex);
}

static void LogAnalyzerThreadData(const loggerData_s* data)
{
    FILE* file = NULL;
    char buffer[MAX_MESSAGE_SIZE] = { 0 };
    cpuLoad_s* cpuLoad = NULL;
    cpuLoad = (cpuLoad_s*)data->message;

    pthread_mutex_lock(&fileMutex);
    file = fopen(FILE_NAME, "a");
    if(file == NULL)
    {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    for(int core = 0; core < systemNumberOfCores; core++)
    {
        size_t ret = 0;
        ret = snprintf(buffer, sizeof(buffer), "Time:%ld THREAD ANALYZER: CORE %2u: %3u%%\n", data->timestamp, cpuLoad[core].core, cpuLoad[core].coreLoadPercentage);
        fwrite(buffer, sizeof(char), ret, file);
        memset(buffer, 0, sizeof(buffer));
    }

    fclose(file);
    pthread_mutex_unlock(&fileMutex);
}


void WriteDebugDataToFile(const loggerData_s* data)
{
    switch(data->threadType)
    {
        case READER:
            LogReaderThreadData(data);
            break;
        
        case ANALYZER:
            LogAnalyzerThreadData(data);
            break;
        
        case PRINTER:
        break;
        
        case WATCHDOG:
        break;

        default:
        break;
    }

}

void LogFrmtdMessageToFile(const char* message, ...)
{
    FILE* file = NULL;
    va_list args;
    char buffer[MAX_MESSAGE_SIZE] = { 0 };
    time_t mytime = time(NULL);
    char* time_str = ctime(&mytime);
    size_t time_str_len = strlen(time_str);
    time_str[time_str_len - 1] = ' ';

    memcpy(buffer, time_str, time_str_len);
    memcpy(buffer + time_str_len, message, strlen(message));

    pthread_mutex_lock(&fileMutex);
    file = fopen(FILE_NAME, "a");
    
    va_start(args, message);
    vfprintf(file, buffer, args);
    va_end(args);

    fclose(file);
    pthread_mutex_unlock(&fileMutex);
}
