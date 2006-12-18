project(kdevqmakeparser)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interface )

add_definitions(-fPIC)

set(kdevqmakeparser_SRCS qmakeast.cpp qmakeparser.cpp)

include(FindFlex)
include(FindBison)

if(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.ll"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -d -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
#     set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
#     set_source_files_properties(
#         ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp
#         GENERATED
#         )
else(FLEX_EXECUTABLE)
    message("Assuming existence of the generated lexer file qmake_lexer.cpp")
	add_custom_command(
		OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.cpp"
		COMMAND ${CP_EXECUTABLE}
		ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.cpp"
		 WORKING_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/"
	)
#     set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
endif(FLEX_EXECUTABLE)

if(BISON_EXECUTABLE)
    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.yy"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        COMMAND ${BISON_EXECUTABLE}
        ARGS    -r all -k -t -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                -d "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.yy"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
#    ADD_CUSTOM_COMMAND(
#        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
#	COMMAND /bin/mv
#	ARGS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
#	"${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCEDIR}
#	)

    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        GENERATED
        )
else(BISON_EXECUTABLE)
    message("Assuming existence of the generated parser file qmake_parser.cpp")
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.hpp"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.hpp"
                "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif(BISON_EXECUTABLE)

kde4_automoc(${kdevqmakeparser_SRCS})
kde4_add_library(kdevqmakeparser SHARED ${kdevqmakeparser_SRCS})
target_link_libraries(kdevqmakeparser ${QT_QTCORE_LIBRARY})

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdevqmakeparser)
#install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})
