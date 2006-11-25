PROJECT(kdevqmakeparser)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interface )

SET(kdevqmakeparser_SRCS)

INCLUDE(FindFlex)
INCLUDE(FindBison)

IF(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.ll"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -d -o"${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp"
                "qmake_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    SET(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
    SET_SOURCE_FILES_PROPERTIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp
        GENERATED
        )
ELSE(FLEX_EXECUTABLE)
    MESSAGE("Assuming existence of the generated lexer file qmake_lexer.cpp")
    SET(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
ENDIF(FLEX_EXECUTABLE)



IF(BISON_EXECUTABLE)
    # Add command to generate the parser.
    ADD_CUSTOM_COMMAND(
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.cpp"
        OUTPUT  "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.yy"
        COMMAND ${BISON_EXECUTABLE}
        ARGS    -d -o"${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.cpp"
                "qmake_parser.yy"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
#    ADD_CUSTOM_COMMAND(
#        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
#	COMMAND /bin/mv
#	ARGS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp" 
#	"${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCEDIR}
#	)

    SET(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
        GENERATED
        )
ELSE(BISON_EXECUTABLE)
    MESSAGE("Assuming existence of the generated parser file qmake_parser.cpp")
    SET(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
ENDIF(BISON_EXECUTABLE)

KDE4_AUTOMOC(${kdevqmakeparser_SRCS})
KDE4_ADD_LIBRARY(kdevqmakeparser STATIC ${kdevqmakeparser_SRCS})

ADD_EXECUTABLE(qmake-parser main.cpp)
TARGET_LINK_LIBRARIES(qmake-parser kdevqmakeparser)
INSTALL(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})
