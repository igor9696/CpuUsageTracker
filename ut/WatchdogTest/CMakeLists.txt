add_executable(WatchdogTest WatchdogTest.cpp)
target_include_directories(WatchdogTest PUBLIC "../../src/Watchdog" "../Stubs")
target_link_libraries(WatchdogTest 
                        WATCHDOG_LIB 
                        GTest::gtest_main
                        GTest::gmock
)


include(GoogleTest)
gtest_discover_tests(WatchdogTest)
