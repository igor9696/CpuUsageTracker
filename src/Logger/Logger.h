#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>
#include <stdio.h>

#define MAX_MESSAGE_SIZE 255

int Logger_Init();
void Logger_DeInit();
void ProcessLogDataToFile();
void LogPrintToFile(const char* message, ...);

#endif // LOGGER_H