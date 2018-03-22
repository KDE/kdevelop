set(astylelib_SRCS
    ASBeautifier.cpp
    ASEnhancer.cpp
    ASFormatter.cpp
    ASResource.cpp
    ASLocalizer.cpp
)
add_library(astylelib STATIC ${astylelib_SRCS})
set_property(TARGET astylelib PROPERTY POSITION_INDEPENDENT_CODE ON)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # astyle lib throws lots of warnings with -Wdocumentation
    target_compile_options(astylelib PRIVATE "-Wno-documentation")
endif()
