
include_directories( ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-fPIC)

########### next target ###############

set(kdevcsharpparser_STAT_SRCS
    decoder.cpp
    decoder.h
    csharp_io.cpp
    csharp_pp_scope.cpp
    csharp_pp_scope.h
    csharp_pp_handler_visitor.cpp
    csharp_pp_handler_visitor.h
    csharp_binder.cpp
    csharp_binder.h
    csharp_typebinder.cpp
    csharp_typebinder.h
    csharp_lexer.h
    codedisplay.cpp
    codedisplay.h
    parsesession.cpp
    parsesession.h
)


# autogenerate the lexer, the two parsers (csharp and csharp_pp)
# and the KDevelop codemodel

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
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp
                --serialize-visitor
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp_pp
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(KDEVPG_EXECUTABLE)
    # If kdev-pg is not available, copy the pre-generated parser files from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_serialize_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"
        # kdev-pg generated files for the csharp parser
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
        # kdev-pg generated files for the csharp pre-processor parser
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        COMMENT "kdev-pg is not installed, using the pre-generated parser files from SVN."
    )
endif(KDEVPG_EXECUTABLE)

set(kdevcsharpparser_STAT_SRCS
    ${kdevcsharpparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
    GENERATED
)


if(FLEX_EXECUTABLE)
    # Copy csharp_lexer.ll to the builddir, so that flex doesn't write out
    # absolute paths in the generated file when we pass them as arguments.
    # In short, I don't want stuff like
    # '#line 2 "/home/kde/build/.../csharp_lexer.cpp" in SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.ll"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.ll"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.ll"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.ll"
    )
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.ll"
        GENERATED
    )
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.ll"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"csharp_lexer.cpp"
                "csharp_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(FLEX_EXECUTABLE)
    # If flex is not available, copy the pre-generated lexer from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_lexer.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_lexer.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
        COMMENT "flex is not installed, using the pre-generated lexer from SVN."
    )
endif(FLEX_EXECUTABLE)

set(kdevcsharpparser_STAT_SRCS
    ${kdevcsharpparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
    GENERATED
)


if(KDEVCMG_EXECUTABLE)
    # Add command to generate the code model
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cm"
        COMMAND ${KDEVCMG_EXECUTABLE}
        ARGS    --language=csharp
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cm"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else(KDEVCMG_EXECUTABLE)
    # If kdev-cmg is not available, copy the pre-generated codemodel files from SVN.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
        COMMAND ${CMAKE_COMMAND}  ARGS -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        COMMENT "kdev-cmg is not installed, using the pre-generated codemodel files from SVN."
    )
endif(KDEVCMG_EXECUTABLE)

set(kdevcsharpparser_STAT_SRCS
    ${kdevcsharpparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
)
set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
    GENERATED
)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})


add_custom_target( copy-generated

# 1. copy actions
    # kdev-pg generated files for the csharp parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_serialize_visitor.h"

    # kdev-pg generated files for the csharp pre-processor parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"

    # flex generated file
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_lexer.cpp"

    # kdev-cmg generated files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_fwd.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"

# 2. depend on the files being copied
    # kdev-pg generated files for the csharp parser
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
    # kdev-pg generated files for the csharp pre-processor parser
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
    # flex generated file
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_lexer.cpp"
    # kdev-cmg generated files
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
            "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
)
