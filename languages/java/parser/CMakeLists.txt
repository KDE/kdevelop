
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )

add_definitions(-fPIC)

########### next target ###############

set(kdevjavaparser_STAT_SRCS
    decoder.cpp
    java_io.cpp
)


# autogenerate the lexer and the parser

FIND_PROGRAM(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")

FIND_PROGRAM(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

IF(KDEVPG_EXECUTABLE)
    # Add command to generate the parser.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/java_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --serialize-visitor
                --output=java
                "java.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevjavaparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.cpp"
        ${kdevjavaparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_SOURCE_DIR}/java_ast.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_serialize_visitor.h"
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("--- Assuming existence of the generated java parser files")
    SET(kdevjavaparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/java_default_visitor.cpp"
        ${kdevjavaparser_STAT_SRCS})
ENDIF(KDEVPG_EXECUTABLE)

IF(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_parser.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"java_lexer.cpp"
                "java_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cpp
        ${kdevjavaparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cpp
        GENERATED
        )
ELSE(FLEX_EXECUTABLE)
    MESSAGE("--- Assuming existence of the generated lexer file java_lexer.cpp")
    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cpp
        ${kdevjavaparser_STAT_SRCS})
ENDIF(FLEX_EXECUTABLE)


kde4_automoc(${kdevjavaparser_STAT_SRCS})
kde4_add_library(kdevjavaparser STATIC ${kdevjavaparser_STAT_SRCS})
