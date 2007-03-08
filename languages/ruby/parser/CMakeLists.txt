
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )

add_definitions(-fPIC)

########### next target ###############

set(kdevrubyparser_STAT_SRCS
    decoder.cpp
    decoder.h
    ruby_io.cpp
    ruby_lexer.h
    parsesession.cpp
    parsesession.h
)


# autogenerate the lexer, the parser and the KDevelop codemodel

find_program(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

find_program(KDEVCMG_EXECUTABLE
    NAMES kdev-cmg
    PATHS /usr/bin
    DOC "kdev-cmg executable")


if(KDEVPG_EXECUTABLE)
    # Add commands to generate the parsers.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/ruby.g"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=ruby
                --serialize-visitor
                --debug-visitor
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(KDEVPG_EXECUTABLE)
    # If kdev-pg is not available, copy the pre-generated parser files from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_serialize_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_debug_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_debug_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
        COMMENT "kdev-pg is not installed, using the pre-generated parser files from SVN."
    )
endif(KDEVPG_EXECUTABLE)

set(kdevrubyparser_STAT_SRCS
    ${kdevrubyparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
    GENERATED
)


if(FLEX_EXECUTABLE)
    # Copy ruby_lexer.ll to the builddir, so that flex doesn't write out
    # absolute paths in the generated file when we pass them as arguments.
    # In short, I don't want stuff like
    # '#line 2 "/home/kde/build/.../ruby_lexer.cpp" in SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.ll"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.ll"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.ll"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.ll"
    )
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.ll"
        GENERATED
    )
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.ll"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"ruby_lexer.cpp"
                "ruby_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(FLEX_EXECUTABLE)
    # If flex is not available, copy the pre-generated lexer from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_lexer.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_lexer.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
        COMMENT "flex is not installed, using the pre-generated lexer from SVN."
    )
endif(FLEX_EXECUTABLE)

set(kdevrubyparser_STAT_SRCS
    ${kdevrubyparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
    GENERATED
)


if(KDEVCMG_EXECUTABLE)
    # Add command to generate the code model
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cm"
        COMMAND ${KDEVCMG_EXECUTABLE}
        ARGS    --language=ruby
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cm"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(KDEVCMG_EXECUTABLE)
    # If kdev-cmg is not available, copy the pre-generated codemodel files from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_fwd.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
        COMMENT "kdev-cmg is not installed, using the pre-generated codemodel files from SVN."
    )
endif(KDEVCMG_EXECUTABLE)

set(kdevrubyparser_STAT_SRCS
    ${kdevrubyparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
    GENERATED
)


kde4_automoc(${kdevrubyparser_STAT_SRCS})
kde4_add_library(kdevrubyparser STATIC ${kdevrubyparser_STAT_SRCS})

# generate the command-line parser
add_executable(ruby-parser main.cpp)
target_link_libraries(ruby-parser kdevrubyparser)
install(TARGETS ruby-parser DESTINATION ${BIN_INSTALL_DIR})


add_custom_target( copy-generated

# 1. copy actions
    # kdev-pg generated files for the ruby parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_default_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_serialize_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_debug_visitor.h"

    # flex generated file
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_lexer.cpp"

    # kdev-cmg generated files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_fwd.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/ruby_codemodel_chameleon.cpp"

# 2. depend on the files being copied
    # kdev-pg generated files for the ruby parser
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ruby_ast.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_parser.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_default_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_serialize_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_debug_visitor.h"
    # flex generated file
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ruby_lexer.cpp"
    # kdev-cmg generated files
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_fwd.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ruby_codemodel_chameleon.cpp"
)
