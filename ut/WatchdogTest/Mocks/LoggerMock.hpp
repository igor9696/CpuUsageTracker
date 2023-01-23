#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;
using ::testing::Return;


class InfLogger
{
public:
    InfLogger() {}
    virtual ~InfLogger() = default;
    virtual void LogPrintToFile(const char* message, ...) = 0;
};

class LoggerMock : public InfLogger
{
public:
    LoggerMock() {};
    virtual ~LoggerMock() {};
    MOCK_METHOD(void, LogPrintToFile, (const char* message, ...), (override));
};



