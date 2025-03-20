cmake_minimum_required(VERSION 3.28)
project(os_course__design C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(os_course__design
        dynamic_memory_management.c
        /page_management.c)
