# astyle lib throws lots of warnings with -Wdocumentation
string(REPLACE "-Wdocumentation" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(astylelib_SRCS
    ASBeautifier.cpp
    ASEnhancer.cpp
    ASFormatter.cpp
    ASResource.cpp
    ASLocalizer.cpp
)
add_library(astylelib STATIC ${astylelib_SRCS})
set_property(TARGET astylelib PROPERTY POSITION_INDEPENDENT_CODE ON)
