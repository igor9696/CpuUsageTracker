#include <stdio.h>

#include "Printer.h"
#include "Analyzer.h"


void PrintFormattedCoreUsage(const cpuLoad_s* LoadData, const uint8_t NumOfCores)
{
    printf("\rCPU core usage:");
    for(int core = 0; core < NumOfCores; core++)
    {
        printf("\tCORE %2u: %3u%%", LoadData[core].core, LoadData[core].coreLoadPercentage);
    }
    fflush(stdout);
}