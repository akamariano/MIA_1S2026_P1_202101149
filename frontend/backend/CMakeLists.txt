cmake_minimum_required(VERSION 3.16)
project(extreamfs)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    core
)

add_executable(extreamfs
    include/main.cpp
    core/commands/mkdisk.cpp
)
