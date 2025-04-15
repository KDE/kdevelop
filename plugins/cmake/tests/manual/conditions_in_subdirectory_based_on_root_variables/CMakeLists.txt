cmake_minimum_required(VERSION 3.10)

set(ROOT_VARIABLE TRUE)

find_package(Qt6Core REQUIRED)
add_subdirectory(subdirectory)

add_executable(root-foo foo.cpp)

if(ROOT_VARIABLE)
    add_executable(root-foo-variable foo.cpp)
endif()

if(Qt6Core_FOUND)
    add_executable(root-foo-package-found foo.cpp)
endif()
