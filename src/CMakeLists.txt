add_subdirectory(Reader)
add_subdirectory(Analyzer)
add_subdirectory(Printer)
add_subdirectory(Logger)
add_subdirectory(Queue)
add_subdirectory(Watchdog)

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
add_executable(${PROJECT_NAME} main.c)
target_link_options(${PROJECT_NAME} PUBLIC -pthread)
target_compile_options(${PROJECT_NAME} PUBLIC -pthread)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -Weverything)
    message(STATUS "Compiler is CLANG")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
    message(STATUS "Compiler is GCC")
endif()


target_link_libraries(${PROJECT_NAME} PUBLIC READER_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC ANALYZER_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC QUEUE_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC PRINTER_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC LOGGER_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC WATCHDOG_LIB)

target_include_directories(${PROJECT_NAME} PUBLIC "Analyzer")
target_include_directories(${PROJECT_NAME} PUBLIC "Logger")
target_include_directories(${PROJECT_NAME} PUBLIC "Printer")
target_include_directories(${PROJECT_NAME} PUBLIC "Queue")
target_include_directories(${PROJECT_NAME} PUBLIC "Reader")
target_include_directories(${PROJECT_NAME} PUBLIC "Watchdog")



