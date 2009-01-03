project(QTestCmakeMultipleSuites)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
enable_testing()

include_directories(${QT_INCLUDES} ${CMAKE_BINARY_DIR})

add_subdirectory(foo)
add_subdirectory(bar)
