#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>


size_t GetNumOfItemsInsideQueue(QueueHandle_t** queue)
{
    return (*queue)->_itemsInQueue;
}

QueueHandle_t* CreateQueue(uint8_t QueueLength, size_t size_of_element)
{
    QueueHandle_t *queue = NULL;
    queue = malloc(sizeof(QueueHandle_t));
    if (queue == NULL)
    {
        return NULL;
    }

    queue->buffer = malloc(size_of_element * QueueLength);
    if (queue->buffer == NULL)
    {
        return NULL;
    }

    queue->_element_size = size_of_element;
    queue->_length = QueueLength;
    queue->_free_idx = 0;
    queue->_alloc_idx = 0;

    sem_init(&(queue->_QueueCntSem), 0, 0);
    pthread_mutex_init(&(queue->_QueueMutex), NULL);
    
    return queue;
}

void DestroyQueue(QueueHandle_t** Queue)
{
    if(*Queue == NULL)
    {
        return;
    }
    sem_destroy(&((*Queue)->_QueueCntSem));
    pthread_mutex_destroy(&((*Queue)->_QueueMutex));

    free((*Queue)->buffer);
    free(*Queue);

    (*Queue)->buffer = NULL;
    *Queue = NULL;
}

int QueueSend(QueueHandle_t** queue, const void* ItemToQueue)
{
    if((*queue)->_length == GetNumOfItemsInsideQueue(queue))
    {
        return 1;
    }

    pthread_mutex_lock(&((*queue)->_QueueMutex));

    char* BufferPtrChar = (char*)(*queue)->buffer;
    size_t BufferAllocShift = ((*queue)->_alloc_idx) * ((*queue)->_element_size);
    memcpy(BufferPtrChar + BufferAllocShift, ItemToQueue, (*queue)->_element_size);

    (*queue)->_itemsInQueue++;
    (*queue)->_alloc_idx = ((*queue)->_alloc_idx + 1) % (*queue)->_length;


    sem_post(&((*queue)->_QueueCntSem));
    pthread_mutex_unlock(&((*queue)->_QueueMutex));
    return 0;
}

void QueueBlockingReceive(QueueHandle_t** queue, void* ItemFromQueue)
{
    // wait until something inside queue
    sem_wait(&((*queue)->_QueueCntSem));
    pthread_mutex_lock(&((*queue)->_QueueMutex));

    // get item from buffer
    char* BufferPtrChar = (char*)(*queue)->buffer;
    size_t BufferAllocShift = ((*queue)->_free_idx) * ((*queue)->_element_size);
    memcpy(ItemFromQueue, BufferPtrChar + BufferAllocShift, (*queue)->_element_size);
    (*queue)->_itemsInQueue--;
    (*queue)->_free_idx = ((*queue)->_free_idx + 1) % (*queue)->_length;

    pthread_mutex_unlock(&((*queue)->_QueueMutex));
}