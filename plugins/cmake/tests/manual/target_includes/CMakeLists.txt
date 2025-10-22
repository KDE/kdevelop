project(target_includes)

add_executable(target main.cpp)
set_property(TARGET target APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/includes")
