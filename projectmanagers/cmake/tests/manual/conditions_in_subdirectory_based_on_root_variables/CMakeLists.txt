set(ROOT_VARIABLE TRUE)

find_package(Qt4 REQUIRED)



add_subdirectory(subdirectory)



add_executable(root-foo foo.cpp)

if(ROOT_VARIABLE)
    add_executable(root-foo-variable foo.cpp)
endif()

if(QT4_FOUND)
    add_executable(root-foo-package-found foo.cpp)
endif()
