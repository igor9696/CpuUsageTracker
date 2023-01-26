set(READER_SRC Reader.c)

add_library(READER_LIB ${READER_SRC})
target_include_directories(READER_LIB PUBLIC "../Logger")
target_include_directories(READER_LIB PUBLIC "../Queue")