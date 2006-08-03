
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )

add_definitions(-fPIC)

########### next target ###############

set(kdevcsharpparser_STAT_SRCS
    decoder.cpp
    csharp_io.cpp
    csharp_pp_scope.cpp
    csharp_pp_handler_visitor.cpp
)


# autogenerate the lexer and the two parsers (csharp and csharp_pp)

FIND_PROGRAM(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")

FIND_PROGRAM(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

IF(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.ll"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"csharp_lexer.cc"
                "csharp_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc)

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc
        GENERATED
        )
ELSE(FLEX_EXECUTABLE)
    MESSAGE("Assuming existence of the generated lexer file csharp_lexer.cc")
    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc)
ENDIF(FLEX_EXECUTABLE)

IF(KDEVPG_EXECUTABLE)
    # Add commands to generate the parsers.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
                ${parser_srcs}
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp
                "csharp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
                ${parser_srcs}
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp_pp
                "csharp_pp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp)

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("Assuming existence of the generated parser files csharp.cpp and csharp_pp.cpp")
    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp)
ENDIF(KDEVPG_EXECUTABLE)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})
