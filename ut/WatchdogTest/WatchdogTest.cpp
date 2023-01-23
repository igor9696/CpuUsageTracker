#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "Watchdog.c"
#include "LoggerStubs.c"
#include "SysCallStubs.c"
}

static int ConstNumOfThreads = 4;

TEST(WatchdogTest, WatchdogInitAllStructMembersAreZerosAndMemoryIsAllocated)
{
    WatchdogInit(ConstNumOfThreads);
        
    EXPECT_EQ(0, Watchdog._head);
    EXPECT_EQ(0, Watchdog._tail);
    EXPECT_EQ(0, Watchdog._registeredThreads);
    EXPECT_TRUE(Watchdog.ThreadsArr != nullptr);

    WatchdogDeinit();
}

TEST(WatchdogTest, WatchdogDeinitReleaseAllocatedMemory)
{
    WatchdogInit(ConstNumOfThreads);
    WatchdogDeinit();

    EXPECT_TRUE(Watchdog.ThreadsArr == nullptr);
}

TEST(WatchdogTest, RegisterUserToWatchdogAndCheckIfInternalStructureIsFilledWithProperData)
{
    WatchdogInit(ConstNumOfThreads);

    WatchdogRegister();
    EXPECT_EQ(1, Watchdog._registeredThreads);
    EXPECT_EQ(777, Watchdog.ThreadsArr[0]._threadID);
    EXPECT_EQ(0, Watchdog.ThreadsArr[0]._internalCnt);
    EXPECT_EQ(0, Watchdog.ThreadsArr[0]._internalPrevCnt);

    WatchdogDeinit();
}

TEST(WatchdogTest, UpdateWatchdogAndVerifyIfInternalCounterisIncremented)
{
    WatchdogInit(ConstNumOfThreads);
    WatchdogRegister();

    EXPECT_EQ(Watchdog.ThreadsArr[0]._internalCnt, Watchdog.ThreadsArr[0]._internalPrevCnt);
    WatchdogUpdate();
    EXPECT_TRUE(Watchdog.ThreadsArr[0]._internalCnt > Watchdog.ThreadsArr[0]._internalPrevCnt);

    WatchdogDeinit();
}

TEST(WatchdogTest, WatchdogCheckAfterWatchdogUpdateAllOK)
{
    int ret = 0;
    WatchdogInit(ConstNumOfThreads);
    WatchdogRegister();
    WatchdogUpdate();

    ret = WatchdogCheck();
    EXPECT_EQ(Watchdog.ThreadsArr[0]._internalCnt, Watchdog.ThreadsArr[0]._internalPrevCnt);
    EXPECT_EQ(0, ret);
}

TEST(WatchdogTest, WatchdogCheckInCaseOfUnupdatedWatchdogInThreadReturnError)
{
    int ret = 0;
    WatchdogInit(ConstNumOfThreads);
    WatchdogRegister();

    ret = WatchdogCheck();
    EXPECT_EQ(-1, ret);
}