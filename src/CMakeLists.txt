add_executable(${PROJECT_NAME} main.c)
target_link_options(${PROJECT_NAME} PUBLIC -pthread)
target_compile_options(${PROJECT_NAME} PUBLIC -pthread)
