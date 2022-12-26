#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

typedef struct QueueHandle_t
{
    uint8_t size;
    void* data_ptr;
} QueueHandle_t;



#endif // QUEUE_H