
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

if(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"ruby_lexer.cpp"
                "ruby_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    set(kdevrubyparser_STAT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.cpp ${kdevrubyparser_STAT_SRCS})
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.cpp
        GENERATED
        )
else(FLEX_EXECUTABLE)
    message("Assuming existence of the generated lexer file ruby_lexer.cpp")
    set(kdevrubyparser_STAT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.cpp ${kdevrubyparser_STAT_SRCS})
endif(FLEX_EXECUTABLE)


if(KDEVPG_EXECUTABLE)
    # Add commands to generate the parsers.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/ruby_ast.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_serialize_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ruby.g"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_lexer.h"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=ruby
                --serialize-visitor
                "ruby.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    set(kdevrubyparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.cpp"
        ${kdevrubyparser_STAT_SRCS})

    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_ast.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_serialize_visitor.h"
        GENERATED
        )
else(KDEVPG_EXECUTABLE)
    message("--- Assuming existence of the generated ruby parser files")
    set(kdevrubyparser_STAT_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_visitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_default_visitor.cpp"
        ${kdevrubyparser_STAT_SRCS})
endif(KDEVPG_EXECUTABLE)


if(KDEVCMG_EXECUTABLE)
    # Add command to generate the code model
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_fwd.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cm"
        COMMAND ${KDEVCMG_EXECUTABLE}
        ARGS    --language=ruby
                "ruby_codemodel.cm"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    set(kdevrubyparser_STAT_SRCS
        ${kdevrubyparser_STAT_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.cpp")

    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_fwd.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.cpp"
        GENERATED
        )
else(KDEVCMG_EXECUTABLE)
    message("--- Assuming existence of the generated ruby codemodel files")
    set(kdevrubyparser_STAT_SRCS
        ${kdevrubyparser_STAT_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ruby_codemodel_chameleon.cpp")
endif(KDEVCMG_EXECUTABLE)


kde4_automoc(kdevrubyparser_STAT ${kdevrubyparser_STAT_SRCS})
kde4_add_library(kdevrubyparser STATIC ${kdevrubyparser_STAT_SRCS})

# generate the command-line parser
add_executable(ruby-parser main.cpp)
target_link_libraries(ruby-parser kdevrubyparser)
install(TARGETS ruby-parser DESTINATION ${BIN_INSTALL_DIR})
