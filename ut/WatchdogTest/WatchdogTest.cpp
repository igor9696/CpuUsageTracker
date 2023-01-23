extern "C" {
#include "Watchdog.c"
}
#include "Mocks/LoggerMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

LoggerMock LoggerMockObj;


TEST(WatchdogTest, Init)
{

    WatchdogInit(1);
    WatchdogRegister();
    WatchdogUpdate();

    EXPECT_EQ(true, false);
}