cmake_minimum_required(VERSION 3.0.0)
project(CUT VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(ut)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CPack)

