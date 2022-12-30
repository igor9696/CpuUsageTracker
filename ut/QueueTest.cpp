extern "C" {
#include "Queue/Queue.h"
}
#include <gtest/gtest.h>


// class QueueTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         queue = CreateQueue(10, sizeof(int));
//     }

//     void TearDown() override {
//         DestroyQueue(&queue);
//     }

//     QueueHandle_t *queue;
// };

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
    
    /* Prepare */
    QueueSend(&queue, )

    /* Excercise */


    int some_val = 69;
}