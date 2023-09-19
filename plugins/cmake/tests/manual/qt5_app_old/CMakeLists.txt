project(testQt5 CXX)
cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME} main.cpp)
qt_use_modules(${PROJECT_NAME} Widgets)
