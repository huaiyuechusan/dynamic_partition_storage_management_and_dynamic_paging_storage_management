cmake_minimum_required(VERSION 3.28)
project(os_course__design C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(os_course__design
        main.c
        dynamic_memory_management.c)
