#ifndef READER_H
#define READER_H

#include <stdint.h>

#define MAX_NUMBER_OF_CORES 4

typedef unsigned long long cputime_t;

typedef struct cpuTimes_s
{
    cputime_t user;
    cputime_t nice;
    cputime_t system;
    cputime_t idle;
    cputime_t iowait;
    cputime_t irq;
    cputime_t softirq;
    cputime_t steal;
    cputime_t guest;
} cpuTimes_s;




#endif // READER_H