#include "Logger.h"
#include "LoggerMock.hpp"

extern LoggerMock LoggerMockObj;

void LogPrintToFile(const char* message, ...)
{
    LoggerMockObj.LogPrintToFile(message, ...); 
}