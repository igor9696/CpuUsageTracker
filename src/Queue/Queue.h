#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct QueueHandle_t
{
    uint8_t _length;
    size_t _element_size;
    size_t _itemsInQueue;
    size_t _alloc_idx;
    size_t _free_idx;
    sem_t _QueueCntSem;
    pthread_mutex_t _QueueMutex;
    void* buffer;

} QueueHandle_t;

QueueHandle_t* CreateQueue(uint8_t QueueLength, size_t size_of_element);
void DestroyQueue(QueueHandle_t** Queue);
int QueueSend(QueueHandle_t** queue, const void* ItemToQueue);
void QueueBlockingReceive(QueueHandle_t** queue, void* ItemFromQueue);
size_t GetNumOfItemsInsideQueue(QueueHandle_t** queue);


#endif // QUEUE_H