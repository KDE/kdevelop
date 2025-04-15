cmake_minimum_required(VERSION 3.10)

project(target_include_directories)

add_library(mylib a.c)
set_property(TARGET mylib PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libincludes")

add_executable(myprogram main.cpp)
target_link_libraries(myprogram mylib)
target_include_directories(myprogram PRIVATE "${CMAKE_SOURCE_DIR}/includes")
