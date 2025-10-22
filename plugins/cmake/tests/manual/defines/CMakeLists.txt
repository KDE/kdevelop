project(defines)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASDF=asdf -DXYZ")
add_executable(target main.cpp)

set_property( DIRECTORY PROPERTY COMPILE_DEFINITIONS A AV=1 )
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS AV2=2 )
set_property( TARGET target PROPERTY COMPILE_DEFINITIONS B BV=1 )
set_property( TARGET target APPEND PROPERTY COMPILE_DEFINITIONS BV2=2 )
set_property( SOURCE main.cpp PROPERTY COMPILE_DEFINITIONS C CV=1 )
set_property( SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS CV2=2 )

add_definitions(-std=c++0x -DBAR=foo)
add_definitions("-DFOO=bar" "-DBLA=blub")
