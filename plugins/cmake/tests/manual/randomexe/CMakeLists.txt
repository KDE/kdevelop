cmake_minimum_required(VERSION 3.10)
project(mytest)

set(EXECUTABLE_OUTPUT_PATH randomplace)
add_executable(mytest main.cpp)

enable_testing()
add_test(mytest mytest)
