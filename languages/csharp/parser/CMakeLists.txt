
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

FIND_PROGRAM(CP_EXECUTABLE
    NAMES cp
    PATHS /bin
    DOC "cp executable")


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
ELSE(KDEVPG_EXECUTABLE)
    # Add commands to copy the generated parser files over to the builddir.
    MESSAGE("--- kdev-pg is not installed to generate csharp parser files: using the ones in SVN")
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_serialize_visitor.h"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_serialize_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/"
        )
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_ast.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_pp_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/"
        )
ENDIF(KDEVPG_EXECUTABLE)

SET(kdevcsharpparser_STAT_SRCS
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_default_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_visitor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_pp_default_visitor.cpp"
    ${kdevcsharpparser_STAT_SRCS})

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
    GENERATED)


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
ELSE(KDEVCMG_EXECUTABLE)
    MESSAGE("--- kdev-cmg is not installed to generate csharp codemodel files: using the ones in SVN")
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/csharp_codemodel_chameleon.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/"
        )
ENDIF(KDEVCMG_EXECUTABLE)

SET(kdevcsharpparser_STAT_SRCS
    ${kdevcsharpparser_STAT_SRCS}
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp")

SET_SOURCE_FILES_PROPERTIES(
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_fwd.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.h"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp_codemodel_chameleon.cpp"
    GENERATED)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})
