cmake_minimum_required(VERSION 3.13.4)

# this is required for cross compilation for some reason
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(pmcs)

file(GLOB SRC_UTIL "src/util/*.c")
file(GLOB SRC_TEST "src/test/*.c")
file(GLOB SRC_MAIN "src/main/*.c")

add_executable(test_runner ${SRC_UTIL} ${SRC_TEST})
add_executable(${PROJECT_NAME} ${SRC_UTIL} ${SRC_MAIN})

