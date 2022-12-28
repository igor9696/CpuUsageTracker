#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>

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

    return queue;
}

void DestroyQueue(QueueHandle_t** Queue)
{
    if(*Queue == NULL)
    {
        return;
    }
    
    free((*Queue)->buffer);
    free(*Queue);

    (*Queue)->buffer = NULL;
    *Queue = NULL;
}