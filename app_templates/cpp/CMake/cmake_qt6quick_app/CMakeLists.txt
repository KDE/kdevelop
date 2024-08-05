cmake_minimum_required(VERSION 3.16)

project(%{APPNAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Qt Quick library
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

add_subdirectory(src)
