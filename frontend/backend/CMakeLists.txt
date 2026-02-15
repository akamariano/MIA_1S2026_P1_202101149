cmake_minimum_required(VERSION 3.16)
project(extreamfs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== INCLUDES =====
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/core
)

# ===== SOURCES =====
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/core/*.cpp
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)

add_executable(extreamfs ${SOURCES})
