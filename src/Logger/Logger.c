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
static QueueHandle_t* logPrintQueue;
static const char* FILE_NAME = "LoggerData.txt";

typedef struct log_t
{
    char message[MAX_MESSAGE_SIZE];
    size_t message_len;
    char* timestamp;
} log_t;

int Logger_Init()
{
    logPrintQueue = CreateQueue(10, sizeof(log_t));
    return pthread_mutex_init(&fileMutex, NULL);
}

void Logger_DeInit()
{
    DestroyQueue(&logPrintQueue);
    pthread_mutex_destroy(&fileMutex);
}

void ProcessLogDataToFile()
{
    FILE* file = NULL;
    log_t logPrint;
    memset(&logPrint, 0, sizeof(log_t));
    QueueBlockingReceive(&logPrintQueue, &logPrint);

    file = fopen(FILE_NAME, "a");
    if (file == NULL)
    {
        return;
    }
    
    fwrite(logPrint.timestamp, sizeof(char), strlen(logPrint.timestamp), file);
    fwrite(logPrint.message, sizeof(char), logPrint.message_len, file);
    fclose(file);
}

// void LogFrmtdMessageToFile(const char* message, ...)
// {
//     FILE* file = NULL;
//     va_list args;
//     char buffer[MAX_MESSAGE_SIZE] = { 0 };
//     time_t mytime = time(NULL);
//     char* time_str = ctime(&mytime);
//     size_t time_str_len = strlen(time_str);
//     time_str[time_str_len - 1] = ' ';

//     memcpy(buffer, time_str, time_str_len);
//     memcpy(buffer + time_str_len, message, strlen(message));

//     pthread_mutex_lock(&fileMutex);
//     file = fopen(FILE_NAME, "a");
    
//     va_start(args, message);
//     vfprintf(file, buffer, args);
//     va_end(args);

//     fclose(file);
//     pthread_mutex_unlock(&fileMutex);
// }

void LogPrintToFile(const char* message, ...)
{
    if(strlen(message) > MAX_MESSAGE_SIZE)
    {
        return;
    }

    va_list args;
    log_t logPrint;
    memset(&logPrint, 0, sizeof(logPrint));
    time_t mytime = time(NULL);

    logPrint.timestamp = ctime(&mytime);
    logPrint.timestamp[strlen(logPrint.timestamp) - 1] = ' ';

    va_start(args, message);
    logPrint.message_len = vsprintf(logPrint.message, message, args);
    va_end(args);

    QueueSend(&logPrintQueue, &logPrint);
}