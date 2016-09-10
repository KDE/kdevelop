cmake_minimum_required(VERSION 2.6)
project(kdev_clang_tidy)

add_executable(kdev_clang_tidy main.cpp)

install(TARGETS kdev_clang_tidy RUNTIME DESTINATION bin)
