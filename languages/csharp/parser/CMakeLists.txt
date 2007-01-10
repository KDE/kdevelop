
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
    csharp_lexer.cpp
    csharp_lexer.h
    codedisplay.cpp
    codedisplay.h
    parsesession.cpp
    parsesession.h
)


# autogenerate the lexer, the two parsers (csharp and csharp_pp)
# and the KDevelop codemodel

FIND_PROGRAM(KDEVPG_EXECUTABLE
    NAMES kdev-pg
    PATHS /usr/bin
    DOC "kdev-pg executable")

FIND_PROGRAM(KDEVCMG_EXECUTABLE
    NAMES kdev-cmg
    PATHS /usr/bin
    DOC "kdev-cmg executable")


IF(KDEVPG_EXECUTABLE)
    # Add commands to generate the parsers.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp
                --serialize-visitor
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp_pp
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        ${kdevcsharpparser_STAT_SRCS}
        )
    SET_SOURCE_FILES_PROPERTIES(
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
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("--- kdev-pg is not installed to generate csharp parser files: using the ones in SVN")
    SET(kdevcsharpparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"
        ${kdevcsharpparser_STAT_SRCS}
        )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/generated )
ENDIF(KDEVPG_EXECUTABLE)


IF(KDEVCMG_EXECUTABLE)
    # Add command to generate the code model
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cm"
        COMMAND ${KDEVCMG_EXECUTABLE}
        ARGS    --language=csharp
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cm"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        )
    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        GENERATED
        )
ELSE(KDEVCMG_EXECUTABLE)
    MESSAGE("--- kdev-cmg is not installed to generate csharp codemodel files: using the ones in SVN")
    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"
        )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/generated )
ENDIF(KDEVCMG_EXECUTABLE)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})


add_custom_target( copy-generated

# 1. copy actions
    # kdev-pg generated files for the csharp parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_serialize_visitor.h"

    # kdev-pg generated files for the csharp pre-processor parser
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"

    # kdev-cmg generated files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_fwd.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"

# 2. depend on the files being copied
    # kdev-pg generated files for the csharp parser
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
    # kdev-pg generated files for the csharp pre-processor parser
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
    # kdev-cmg generated files
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
)
