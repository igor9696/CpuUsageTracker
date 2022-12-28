extern "C" {
#include "Queue/Queue.h"
}
#include <gtest/gtest.h>


TEST(QueueTest, CreateQueueAndGetValidPointers)
{
    /*Init*/
    QueueHandle_t *queue = NULL;

    /* Excercise */
    queue = CreateQueue(10, sizeof(int));

    /* Verify */
    EXPECT_TRUE(queue != nullptr);
    EXPECT_TRUE(queue->buffer != nullptr);
}

TEST(QueueTest, CreateQueueAndCheckIfInternalStruckVariablesAreCorrect)
{
    /*Init*/
    QueueHandle_t *queue = NULL;

    /* Excercise */
    queue = CreateQueue(10, sizeof(int));

    /* Verify */
    EXPECT_EQ(10, queue->_length);
    EXPECT_EQ(sizeof(int), queue->_element_size);
}


TEST(QueueTest, DestroyQueueAndGetNullPointers)
{
    /*Init*/
    QueueHandle_t *queue = CreateQueue(10, sizeof(int));

    /* Excercise */
    DestroyQueue(&queue);

    /* Verify */
    EXPECT_TRUE(queue->buffer == NULL);
    EXPECT_TRUE(queue == NULL);
}