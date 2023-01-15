#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include "../Logger/Logger.h"

size_t GetNumOfItemsInsideQueue(QueueHandle_t** queue)
{
    return (*queue)->_itemsInQueue;
}

QueueHandle_t* CreateQueue(uint8_t QueueLength, size_t size_of_element)
{
    QueueHandle_t *queue = NULL;
    queue = calloc(1, sizeof(QueueHandle_t));
    if (queue == NULL)
    {
        return NULL;
    }

    queue->buffer = calloc(1, (size_of_element * QueueLength));
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
    *Queue = NULL;
}

int QueueSend(QueueHandle_t** queue, const void* ItemToQueue)
{
    if(*queue == NULL)
    {
        return -1;
    }

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
    if(*queue == NULL)
    {
        return;
    }

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

int QueueBlockingReceiveTimeout(QueueHandle_t** queue, void* ItemFromQueue, size_t timeout_sec)
{
    if(*queue == NULL)
    {
        LogPrintToFile("FUNC:%s Msg: NULL pointer received!\n", __FUNCTION__);
        return -1;
    }

    struct timespec ts = { 0 };
    if(-1 == clock_gettime(CLOCK_REALTIME, &ts))
    {
        LogPrintToFile("FUNC:%s Msg: Error in clock_gettime function!\n", __FUNCTION__);
        return -1;
    }
    ts.tv_sec += timeout_sec;

    // wait timeout_sec untill something appear in queue, if not return
    if(-1 == sem_timedwait(&((*queue)->_QueueCntSem), &ts))
    {
        LogPrintToFile("FUNC:%s Msg: Timeout expired in queue wait\n", __FUNCTION__);
        return -1;
    }
    pthread_mutex_lock(&((*queue)->_QueueMutex));

    char* BufferPtrChar = (char*)(*queue)->buffer;
    size_t BufferAllocShift = ((*queue)->_free_idx) * ((*queue)->_element_size);
    memcpy(ItemFromQueue, BufferPtrChar + BufferAllocShift, (*queue)->_element_size);
    (*queue)->_itemsInQueue--;
    (*queue)->_free_idx = ((*queue)->_free_idx + 1) % (*queue)->_length;

    pthread_mutex_unlock(&((*queue)->_QueueMutex));
    return 0;
}



int QueueNonBlockingReceive(QueueHandle_t** queue, void* ItemFromQueue)
{
    if(*queue == NULL)
    {
        return -1;
    }

    if(GetNumOfItemsInsideQueue(queue) > 0)
    {
        if(pthread_mutex_trylock(&((*queue)->_QueueMutex)) == 0)
        {
            char* BufferPtrChar = (char*)(*queue)->buffer;
            size_t BufferAllocShift = ((*queue)->_free_idx) * ((*queue)->_element_size);
            memcpy(ItemFromQueue, BufferPtrChar + BufferAllocShift, (*queue)->_element_size);

            (*queue)->_itemsInQueue--;
            (*queue)->_free_idx = ((*queue)->_free_idx + 1) % (*queue)->_length;
            pthread_mutex_unlock(&((*queue)->_QueueMutex));
            return 0;
        }
        else
        {
            return -1;
        }
    }

    return -1;
}