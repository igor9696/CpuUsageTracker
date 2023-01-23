add_library(LoggerMock SHARED LoggerMock.cpp)
target_include_directories(LoggerMock PUBLIC "../../../src/Logger")

target_link_libraries(LoggerMock  
                        GTest::gtest_main
                        GTest::gmock
)

include(GoogleTest)
