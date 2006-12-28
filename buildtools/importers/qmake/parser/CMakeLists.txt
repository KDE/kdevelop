project(kdevqmakeparser)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interface
                     ${CMAKE_SOURCE_DIR}/buildtools/importers/qmake
                   )

set(kdevqmakeparser_SRCS qmakeast.cpp qmakedriver.cpp)

include(FindFlex)
include(FindBison)

if(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -d -s -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp
        GENERATED
    )
else(FLEX_EXECUTABLE)
    message("Assuming existence of the generated lexer file qmake_lexer.cpp")
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp
        GENERATED
    )

    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.cpp"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_lexer.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/"
    )
endif(FLEX_EXECUTABLE)

if(BISON_EXECUTABLE)
    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
                "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
                "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
                "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.yy"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        COMMAND ${BISON_EXECUTABLE}
        ARGS    -r all -k -t -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                -d "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.yy"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
        GENERATED
    )
else(BISON_EXECUTABLE)
    message("Assuming existence of the generated parser file qmake_parser.cpp")
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
        GENERATED
    )
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/location.hh"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/position.hh"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/stack.h"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/location.hh"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/position.hh"
                "${CMAKE_CURRENT_SOURCE_DIR}/generated/stack.h"
                "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/"
    )
endif(BISON_EXECUTABLE)

kde4_automoc(${kdevqmakeparser_SRCS})
kde4_add_library(kdevqmakeparser SHARED ${kdevqmakeparser_SRCS})
target_link_libraries(kdevqmakeparser ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})
install(TARGETS kdevqmakeparser DESTINATION ${LIB_INSTALL_DIR} )

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdevqmakeparser)
#install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})
