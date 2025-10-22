project(conditions_in_subdirectory_based_on_root_variables)

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
