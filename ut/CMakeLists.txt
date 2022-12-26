set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(QueueTest QueueTest.cpp)
target_link_libraries(QueueTest GTest::gtest_main QUEUE_LIB)
include(GoogleTest)
gtest_discover_tests(QueueTest)

