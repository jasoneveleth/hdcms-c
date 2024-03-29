cmake_minimum_required(VERSION 3.13.4)


IF(WIN32)
    # std:c17 (or something recent) is needed for `noreturn`
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /std:c17 /O2 /Zi")
    # 4820: struct padding, 4068: unknown pragma, 5045: spectre mitigation,
    # 4996: unsafe system calls, 4464: '..' in include path, 
    # 4706: assignment in conditional
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4820 /wd4068 /wd5045 /wd4996 /wd4464 /wd4706")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/data/\\\"")
ELSE()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O2 -fno-omit-frame-pointer -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/data/\\\"")
ENDIF()

# Make excecutables output into current directory in both debug and release build modes (default on unix)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ".")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ".")

project(high_dimensional_consensus_mass_spec)
set(TEST "test_runner")
set(MAIN "hdcms")

file(GLOB SRC_UTIL "src/util/*.c")
file(GLOB SRC_TEST "src/test.c")
file(GLOB SRC_MAIN "src/main.c")

add_executable(${TEST} ${SRC_UTIL} ${SRC_TEST})
add_executable(${MAIN} ${SRC_UTIL} ${SRC_MAIN})


# link with math library
find_library(MATHLIB m)
if(MATHLIB)
    target_link_libraries(${TEST} PUBLIC m)
    target_link_libraries(${MAIN} PUBLIC m)
endif()

