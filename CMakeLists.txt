cmake_minimum_required(VERSION 3.13.4)

# this is required for cross compilation for some reason
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(test_runner)

add_executable(test_runner src/array.c src/peak.c test/data.c test/test.c)
