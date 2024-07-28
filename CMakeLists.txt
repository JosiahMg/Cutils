cmake_minimum_required(VERSION 3.13)

project(cutils)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Werror")
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB C_SRC_FILES "*.c")

add_executable(cutils ${C_SRC_FILES})
