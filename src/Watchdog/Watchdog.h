#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <unistd.h>

int WatchdogInit(const int NumOfThreads);
void WatchdogDeinit();
void WatchdogRegister();
void WatchdogUpdate();
int WatchdogCheck();

#endif // WATCHDOG_H