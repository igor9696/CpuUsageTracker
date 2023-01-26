add_library(ANALYZER_LIB Analyzer.c)

target_include_directories(ANALYZER_LIB PUBLIC "../Logger")
target_include_directories(ANALYZER_LIB PUBLIC "../Reader")
target_include_directories(ANALYZER_LIB PUBLIC "../Queue")