
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
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/java.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/java.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java.g"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --adapt-to=kdevelop
                --output=java
                "java.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
        ${kdevjavaparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/java.h
        GENERATED
        )
ELSE(KDEVPG_EXECUTABLE)
    MESSAGE("Assuming existence of the generated parser files java.[h,cpp]")
    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
        ${kdevjavaparser_STAT_SRCS})
ENDIF(KDEVPG_EXECUTABLE)

IF(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cc"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/java.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -o"${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cc"
                "${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.ll"
        )

    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cc
        ${kdevjavaparser_STAT_SRCS})

    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cc
        GENERATED
        )
ELSE(FLEX_EXECUTABLE)
    MESSAGE("Assuming existence of the generated lexer file java_lexer.cc")
    SET(kdevjavaparser_STAT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/java_lexer.cc
        ${kdevjavaparser_STAT_SRCS})
ENDIF(FLEX_EXECUTABLE)


kde4_automoc(${kdevjavaparser_STAT_SRCS})
kde4_add_library(kdevjavaparser STATIC ${kdevjavaparser_STAT_SRCS})
