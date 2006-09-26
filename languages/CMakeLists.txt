
add_subdirectory(cpp)


# only build Flex-dependent language parts if there's a reasonably current
# version installed which supports the Flex' C++ lexer classes we are using.
set(FLEX_REQUIRED_VERSION "2.5.31")

find_package(Flex)

if(FLEX_FOUND)
    macro_ensure_version("2.5.31" "${FLEX_VERSION}" FLEX_VERSION_OK)
endif(FLEX_FOUND)

if(FLEX_VERSION_OK)
#    add_subdirectory(fake)
    add_subdirectory(java)
    add_subdirectory(csharp)
    add_subdirectory(ruby)
else(FLEX_VERSION_OK)
    message(STATUS "Flex version ${FLEX_REQUIRED_VERSION} or higher required, but not installed:")
    message(STATUS "The java, csharp and ruby language parts will not be built!")
endif(FLEX_VERSION_OK)
