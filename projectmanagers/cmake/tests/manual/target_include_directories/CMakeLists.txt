cmake_minimum_required(VERSION 2.8)

add_executable(myprogram main.cpp)
target_include_directories(myprogram PRIVATE "${CMAKE_SOURCE_DIR}/includes")
