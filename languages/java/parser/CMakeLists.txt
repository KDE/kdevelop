
include_directories( ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-fPIC)

########### next target ###############

set(kdevjavaparser_STAT_SRCS
    decoder.cpp
    decoder.h
    java_io.cpp
    java_lexer.cpp
    java_lexer.h
    parsesession.cpp
    parsesession.h
)


# autogenerate the lexer and the parser

FIND_PROGRAM(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

IF(KDEVPG_EXECUTABLE)
    # Add command to generate the parser.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --serialize-visitor
                --output=java
                "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    SET(kdevjavaparser_STAT_SRCS
        "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
        ${kdevjavaparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
        "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
        "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
        "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
        "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("--- Assuming existence of the generated java parser files")
    SET(kdevjavaparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.cpp"
        ${kdevjavaparser_STAT_SRCS})
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/generated )
ENDIF(KDEVPG_EXECUTABLE)


kde4_automoc(${kdevjavaparser_STAT_SRCS})
kde4_add_library(kdevjavaparser STATIC ${kdevjavaparser_STAT_SRCS})


add_custom_target( copy-generated

# 1. copy the kdev-pg generated files for the java parser
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_ast.h"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.h"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.cpp"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.h"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.cpp"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.h"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.cpp"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_serialize_visitor.h"

# 2. depend on the files being copied
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
)
