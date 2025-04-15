cmake_minimum_required(VERSION 3.10)

project(qt6_app)

find_package(Qt6 REQUIRED Widgets)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram Qt::Widgets)
