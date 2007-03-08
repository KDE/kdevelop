
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
    java_lexer.h
    parsesession.cpp
    parsesession.h
)


# autogenerate the lexer and the parser

find_program(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

if(KDEVPG_EXECUTABLE)
    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --serialize-visitor
                --output=java
                "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(KDEVPG_EXECUTABLE)
    # If kdev-pg is not available, copy the pre-generated parser files from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_serialize_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
        COMMENT "kdev-pg is not installed, using the pre-generated parser files from SVN."
    )
endif(KDEVPG_EXECUTABLE)

set(kdevjavaparser_STAT_SRCS
    ${kdevjavaparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
)
set_source_files_properties(
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


if(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.ll"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.ll"
    )
else(FLEX_EXECUTABLE)
    # If flex is not available, copy the pre-generated lexer from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_lexer.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_lexer.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp"
        COMMENT "flex is not installed, using the pre-generated lexer from SVN."
    )
endif(FLEX_EXECUTABLE)

set(kdevjavaparser_STAT_SRCS
    ${kdevjavaparser_STAT_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp
)
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/java_lexer.cpp
    GENERATED
)


kde4_automoc(${kdevjavaparser_STAT_SRCS})
kde4_add_library(kdevjavaparser STATIC ${kdevjavaparser_STAT_SRCS})


add_custom_target( copy-generated

# 1. copy the kdev-pg generated files for the java parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_default_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/java_serialize_visitor.h"

# 2. depend on the files being copied
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/java_ast.h"
            "${CMAKE_CURRENT_BINARY_DIR}/java_parser.h"
            "${CMAKE_CURRENT_BINARY_DIR}/java_parser.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/java_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/java_default_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/java_serialize_visitor.h"
)
