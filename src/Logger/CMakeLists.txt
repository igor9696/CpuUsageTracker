add_library(LOGGER_LIB Logger.c)

target_include_directories(LOGGER_LIB PUBLIC "../Analyzer")
target_include_directories(LOGGER_LIB PUBLIC "../Reader")
target_include_directories(LOGGER_LIB PUBLIC "../Queue")
