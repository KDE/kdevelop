cmake_minimum_required(VERSION 3.10)

project(target_includes)

add_executable(target main.cpp)
set_property(TARGET target APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/includes")
