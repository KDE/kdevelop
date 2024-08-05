add_executable(subdirectory-foo foo.cpp)

if(ROOT_VARIABLE)
    add_executable(subdirectory-foo-variable foo.cpp)
endif()

if(Qt6Core_FOUND)
    add_executable(subdirectory-foo-package-found foo.cpp)
endif()
