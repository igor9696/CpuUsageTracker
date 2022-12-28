#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct QueueHandle_t
{
    uint8_t _length;
    size_t _element_size;
    void* buffer;
} QueueHandle_t;

QueueHandle_t* CreateQueue(uint8_t QueueLength, size_t size_of_element);
void DestroyQueue(QueueHandle_t** Queue);
void QueueSend(QueueHandle_t** queue, const void* ItemToQueue);


#endif // QUEUE_H