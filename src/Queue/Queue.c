#include "Queue.h"
#include <stdlib.h>



QueueHandle_t* CreateQueue(uint8_t QueueLength, size_t size_of_element)
{
    QueueHandle_t *s = NULL;
    s = (QueueHandle_t*)malloc(size_of_element * QueueLength);
    return s;
}

void DestroyQueue(QueueHandle_t* queue)
{
    free(queue);
}