cmake_minimum_required(VERSION 2.8)

find_package(Qt5 REQUIRED Widgets)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram Qt${QT_MAJOR_VERSION}::Widgets)
