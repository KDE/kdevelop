
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

IF(KDEVPG_EXECUTABLE)
    # Add commands to generate the parsers.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp
                "csharp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp_pp
                "csharp_pp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp
        ${kdevcsharpparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.h
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("Assuming existence of the generated parser files csharp.[h,cpp] and csharp_pp.[h,cpp]")
    SET(kdevcsharpparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.h
        ${kdevcsharpparser_STAT_SRCS})
ENDIF(KDEVPG_EXECUTABLE)

IF(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"csharp_lexer.cc"
                "csharp_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc
        ${kdevcsharpparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc
        GENERATED
        )
ELSE(FLEX_EXECUTABLE)
    MESSAGE("Assuming existence of the generated lexer file csharp_lexer.cc")
    SET(kdevcsharpparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cc
        ${kdevcsharpparser_STAT_SRCS})
ENDIF(FLEX_EXECUTABLE)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})
