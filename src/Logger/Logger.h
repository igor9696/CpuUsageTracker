#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>
#include <stdio.h>

#define MAX_MESSAGE_SIZE 255
static const char* FILE_NAME = "LoggerData.txt";

typedef enum threadType
{
    READER,
    ANALYZER,
    PRINTER,
    WATCHDOG,
} threadType_e;

typedef struct logger_s
{
    threadType_e threadType;
    size_t message_size;
    const void* message;
    time_t timestamp; // seconds since January 1970
} loggerData_s;

int Logger_Init();
void Logger_DeInit();
void WriteDebugDataToFile(const loggerData_s* data);
void LogFrmtdMessageToFile(const char* message, ...);

#endif // LOGGER_H