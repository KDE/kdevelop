project(target_includes)

add_executable(target main.cpp)
set_property(TARGET target APPEND PROPERTY COMPILE_DEFINITIONS "VALUE=1")
