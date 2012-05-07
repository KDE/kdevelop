cmake_minimum_required(VERSION 2.8.8)

project(target_includes)

add_executable(target main.cpp)
set_property(TARGET target APPEND PROPERTY INCLUDE_DIRECTORIES "includes")
