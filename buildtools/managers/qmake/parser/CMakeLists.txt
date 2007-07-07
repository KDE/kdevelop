project(kdevqmakeparser)

include_directories( ${KDEVPLATFORM_INCLUDE_DIR} ${KDEVPLATFORM_INCLUDE_DIR}/interface
                     ${CMAKE_SOURCE_DIR}/buildtools/managers/qmake
                   )

set( kdevqmakeparser_SRCS
     qmakeast.cpp
     qmakeastassignment.cpp
     qmakeastbody.cpp
     qmakeastor.cpp
     qmakeastproject.cpp
     qmakeastfunc.cpp
     qmakeastscope.cpp
     qmakeastsimple.cpp
     qmakedriver.cpp
    )

include(FindFlex)
include(FindBison)
include(MacroEnsureVersion)

if(BISON_FOUND)
    macro_ensure_version2( "2.3" "${BISON_VERSION}" BISON_NOT_OLD )
endif(BISON_FOUND)
if(FLEX_FOUND)
    macro_ensure_version( "2.5.31" "${FLEX_VERSION}" FLEX_NOT_OLD )
endif(FLEX_FOUND)

if(BISON_EXECUTABLE AND FLEX_EXECUTABLE AND BISON_NOT_OLD AND FLEX_NOT_OLD)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.ll"
                "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.h"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -L -s -o"${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp
        GENERATED
    )

    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.hpp"
                "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
                "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
                "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmakeparser.yy"
                "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"
        COMMAND ${BISON_EXECUTABLE}
        ARGS    -l -r none -o"${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp"
                -d "${CMAKE_CURRENT_SOURCE_DIR}/qmakeparser.yy"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
        GENERATED
    )

    add_custom_target( debug-parser
        COMMAND ${FLEX_EXECUTABLE} -s -d -o "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"   "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.ll"
        COMMAND ${BISON_EXECUTABLE} -r all -k -t -o"${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp" -d "${CMAKE_CURRENT_SOURCE_DIR}/qmakeparser.yy"

        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmakeparser.yy"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.ll"
        )
    include_directories( ${CMAKE_CURRENT_BINARY_DIR} 
                     ${FLEX_INCLUDE_DIR} )

else(BISON_EXECUTABLE AND FLEX_EXECUTABLE AND BISON_NOT_OLD AND FLEX_NOT_OLD)
    message(STATUS "Assuming existence of the generated lexer file qmakelexer.cpp")
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakelexer.cpp ${kdevqmakeparser_SRCS})
    message(STATUS "Assuming existence of the generated parser file qmakeparser.cpp")
    set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeparser.cpp ${kdevqmakeparser_SRCS})
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/generated )
endif(BISON_EXECUTABLE AND FLEX_EXECUTABLE AND BISON_NOT_OLD AND FLEX_NOT_OLD)

kde4_automoc(${kdevqmakeparser_SRCS})
kde4_add_library(kdevqmakeparser SHARED ${kdevqmakeparser_SRCS})
target_link_libraries(kdevqmakeparser ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})
set_target_properties(kdevqmakeparser PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS kdevqmakeparser DESTINATION ${LIB_INSTALL_DIR} )

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdevqmakeparser)
#install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})

add_custom_target( copy-generated
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeparser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeparser.hpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/location.hh" "${CMAKE_CURRENT_SOURCE_DIR}/generated/location.hh"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/stack.hh" "${CMAKE_CURRENT_SOURCE_DIR}/generated/stack.hh"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/position.hh" "${CMAKE_CURRENT_SOURCE_DIR}/generated/position.hh"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakelexer.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.hpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/location.hh"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/stack.hh"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/position.hh"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"
    )
