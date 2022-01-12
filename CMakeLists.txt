cmake_minimum_required(VERSION 3.13.4)

# this is required for cross compilation for some reason
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(pmcs)
set(TEST "test_runner")

file(GLOB SRC_UTIL "src/util/*.c")
file(GLOB SRC_TEST "src/test/*.c")
file(GLOB SRC_MAIN "src/main/*.c")

add_executable(${TEST} ${SRC_UTIL} ${SRC_TEST})
# link with math library
target_link_libraries(${TEST} PUBLIC m)

add_executable(${PROJECT_NAME} ${SRC_UTIL} ${SRC_MAIN})
# link with math library
target_link_libraries(${PROJECT_NAME} PUBLIC m)

