add_library(QUEUE_LIB SHARED Queue.c)
target_link_libraries(QUEUE_LIB LOGGER_LIB)