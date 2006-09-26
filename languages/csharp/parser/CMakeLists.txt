
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )

add_definitions(-fPIC)

########### next target ###############

set(kdevcsharpparser_STAT_SRCS
    decoder.cpp
    csharp_io.cpp
    csharp_pp_scope.cpp
    csharp_pp_handler_visitor.cpp
    csharp_binder.cpp
    csharp_typebinder.cpp
    codedisplay.cpp
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
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp
                --serialize-visitor
                "csharp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=csharp_pp
                "csharp_pp.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.cpp"
        ${kdevcsharpparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_ast.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_serialize_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_ast.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.cpp"
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("--- Assuming existence of the generated csharp parser files")
    SET(kdevcsharpparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_default_visitor.cpp"
        ${kdevcsharpparser_STAT_SRCS})
ENDIF(KDEVPG_EXECUTABLE)


# Add command to generate the lexer.
ADD_CUSTOM_COMMAND(
    OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.ll"
            "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/csharp_parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/csharp_pp_parser.h"
    COMMAND ${FLEX_EXECUTABLE}
    ARGS    -o"csharp_lexer.cpp"
            "csharp_lexer.ll"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

SET(kdevcsharpparser_STAT_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cpp"
    ${kdevcsharpparser_STAT_SRCS})

SET_SOURCE_FILES_PROPERTIES(
    "${CMAKE_CURRENT_SOURCE_DIR}/csharp_lexer.cpp"
    GENERATED
    )
# ...now the lexer has been generated


IF(KDEVCMG_EXECUTABLE)
    # Add command to generate the code model
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_fwd.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cm"
        COMMAND ${KDEVCMG_EXECUTABLE}
        ARGS    --language=csharp
                "csharp_codemodel.cm"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.cpp")

    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_fwd.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.cpp"
        GENERATED
        )
ELSE(KDEVCMG_EXECUTABLE)
    MESSAGE("--- Assuming existence of the generated csharp codemodel files")
    SET(kdevcsharpparser_STAT_SRCS
        ${kdevcsharpparser_STAT_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/csharp_codemodel_chameleon.cpp")
ENDIF(KDEVCMG_EXECUTABLE)


kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})
