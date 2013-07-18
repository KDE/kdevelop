cmake_minimum_required(VERSION 2.8)

find_package(Qt5 REQUIRED Core)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram Qt5::Core)
