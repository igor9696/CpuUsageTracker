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
    EXPECT_EQ(queue->_alloc_idx, 0);
    EXPECT_EQ(queue->_free_idx, 0);
    EXPECT_EQ(queue->_itemsInQueue, 0);
}


TEST(QueueTest, DestroyQueueAndGetNullPointers)
{
    /*Init*/
    QueueHandle_t *queue = CreateQueue(10, sizeof(int));

    /* Excercise */
    DestroyQueue(&queue);

    /* Verify */
    EXPECT_TRUE(queue == NULL);
}

TEST(QueueTest, SendElementToEmptyQueue)
{
    /*Init*/
    QueueHandle_t *queue = NULL;
    queue = CreateQueue(10, sizeof(int));
    int val = 69;

    /* Excercise */
    QueueSend(&queue, (const void*)&val);
    
    /* Verify */
    EXPECT_EQ(val, *(int*)(queue->buffer));
}

TEST(QueueTest, SendElementToQueueAndGetBiggerQueueItemCounter)
{
    /*Init*/
    QueueHandle_t *queue = CreateQueue(10, sizeof(int));
    int some_val = 69;


    /* Excercise */
    EXPECT_EQ(0, GetNumOfItemsInsideQueue(&queue));
    
    QueueSend(&queue, (const void*)&some_val);
    QueueSend(&queue, (const void*)&some_val);
    
    /* Verify */
    EXPECT_EQ(2, GetNumOfItemsInsideQueue(&queue));
}

TEST(QueueTest, ReceiveElementFromQueueAndGetSmallerQueueItemCounter)
{
     /*Init*/
    QueueHandle_t *queue = NULL;
    queue = CreateQueue(10, sizeof(int));
    int val = 69;
    int ret = 0;

    /* Prepare */
    QueueSend(&queue, (const void*)&val);
    EXPECT_EQ(1, GetNumOfItemsInsideQueue(&queue));

    /* Excercise */
    QueueBlockingReceive(&queue, (void*)&ret);
    EXPECT_EQ(0, GetNumOfItemsInsideQueue(&queue));

    /* Verify */
    EXPECT_EQ(ret, val);
}

TEST(QueueTest, SendThreeElementsToQueueAndReceiveThemInProperOrder)
{
     /*Init*/
    QueueHandle_t *queue = NULL;
    queue = CreateQueue(10, sizeof(int));
    int val1, val2, val3;
    val1 = 10; val2 = 22; val3 = 44;
    int ret = 0;

    /* Prepare */
    QueueSend(&queue, (const void*)&val1);
    QueueSend(&queue, (const void*)&val2);
    QueueSend(&queue, (const void*)&val3);

    /* Excercise */
    QueueBlockingReceive(&queue, (void*)&ret);
    EXPECT_EQ(ret, val1);

    QueueBlockingReceive(&queue, (void*)&ret);
    EXPECT_EQ(ret, val2);

    QueueBlockingReceive(&queue, (void*)&ret);
    EXPECT_EQ(ret, val3);

    EXPECT_EQ(0, GetNumOfItemsInsideQueue(&queue));
}

TEST(QueueTest, TryToSendElementToBusyQueueAndReturnWithErrorCode)
{
    /*Init*/
    QueueHandle_t *queue = CreateQueue(3, sizeof(int));
    int val = 100;
    int ret = -1;

    /* Prepare */
    ret = QueueSend(&queue, (const void*)&val);
    ret = QueueSend(&queue, (const void*)&val);
    ret = QueueSend(&queue, (const void*)&val);
    
    EXPECT_EQ(0, ret);
    
    /* Verify */
    ret = QueueSend(&queue, (const void*)&val);
    EXPECT_EQ(1, ret);
}

TEST(QueueTest, FillOutQueue_ReceiveAllElements_FillItOutAgain)
{
    /*Init*/
    QueueHandle_t *queue = CreateQueue(3, sizeof(int));
    int val1 = 100;
    int val2 = 200;
    int val3 = 300;
    int ret = -1;
    int output = 0;

    /* Prepare */
    /* Fill out queue */
    ret = QueueSend(&queue, (const void*)&val1);    
    ret = QueueSend(&queue, (const void*)&val2);
    ret = QueueSend(&queue, (const void*)&val3);

    /* Receive all elements */
    QueueBlockingReceive(&queue, (void*)&output);
    QueueBlockingReceive(&queue, (void*)&output);
    QueueBlockingReceive(&queue, (void*)&output);

    /* Fill again */
    ret = QueueSend(&queue, (const void*)&val1);    
    ret = QueueSend(&queue, (const void*)&val2);
    ret = QueueSend(&queue, (const void*)&val3);

    QueueBlockingReceive(&queue, (void*)&output);
    EXPECT_EQ(val1, output);
    QueueBlockingReceive(&queue, (void*)&output);
    EXPECT_EQ(val2, output);    
    QueueBlockingReceive(&queue, (void*)&output);
    EXPECT_EQ(val3, output);
}


TEST(QueueTest, PutStructureToQueueAndReceiveIt)
{
    /* Init */
    struct Entity {
        int val1;
        float val2;
        char val3;
    };

    struct Entity e = {
        .val1 = 69,
        .val2 = 33.5,
        .val3 = 'C'
    };

    struct Entity ret = { 0 };
    QueueHandle_t* queue = CreateQueue(4, sizeof(struct Entity));

    /* Excersice */
    QueueSend(&queue, (const void*)&e);
    QueueBlockingReceive(&queue, (void*)&ret);

    /* Verify */
    EXPECT_EQ(e.val1, ret.val1);
    EXPECT_EQ(e.val2, ret.val2);
    EXPECT_EQ(e.val3, ret.val3);
}


