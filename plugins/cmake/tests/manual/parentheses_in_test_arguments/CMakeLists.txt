cmake_minimum_required(VERSION 3.10)

PROJECT(with_build_dir)
add_executable(main main.cpp)

enable_testing()
add_test(sometestname main "(foo)" "bar")
