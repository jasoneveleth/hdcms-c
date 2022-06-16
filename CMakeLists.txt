cmake_minimum_required(VERSION 3.13.4)

# this is required for cross compilation for some reason
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# compiler
# set(CMAKE_C_COMPILER /usr/bin/clang)
IF (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /RTCs /O2 /std:c17")
ELSE()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c17 -Wall -Wextra -Wpedantic -O2 -fno-omit-frame-pointer -g")
ENDIF()

project(prob_mass_spec)
set(TEST "test_runner")

file(GLOB SRC_UTIL "src/util/*.c")
file(GLOB SRC_TEST "src/test/*.c")
file(GLOB SRC_MAIN "src/main/*.c")

add_executable(${TEST} ${SRC_UTIL} ${SRC_TEST})
add_executable(${PROJECT_NAME} ${SRC_UTIL} ${SRC_MAIN})

# link with math library
find_library(MATHLIB m)
if(MATHLIB)
    target_link_libraries(${TEST} PUBLIC m)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

