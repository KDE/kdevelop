
project(kdevqmakeparser)

include_directories( ${KDEVPLATFORM_INCLUDE_DIR} ${KDEVPLATFORM_INCLUDE_DIR}/interface )

set(kdevqmakeparser_SRCS
    qmakeast.cpp
    qmakeastassignment.cpp
    qmakeastbody.cpp
    qmakeastor.cpp
    qmakeastproject.cpp
    qmakeastfunc.cpp
    qmakeastscope.cpp
    qmakeastsimple.cpp
    qmakedriver.cpp
    qmakelexer.cpp
    )

find_package(KDevelop-PG QUIET)

if(KDEVPG_FOUND)
    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake.g"
                "${CMAKE_CURRENT_BINARY_DIR}/qmakelexer.cpp"
        COMMAND ${KDEVPG_EXECUTABLE}
        ARGS    --output=qmake "${CMAKE_CURRENT_SOURCE_DIR}/qmake.g"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(kdevqmakeparser_SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.cpp
        ${kdevqmakeparser_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
        GENERATED
    )

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDEVPG_INCLUDE_DIR} )

else(KDEVPG_FOUND)
    message(STATUS "Assuming existence of the generated parser files for the parser")
    set(kdevqmakeparser_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_default_visitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_visitor.cpp
        ${kdevqmakeparser_SRCS} )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/generated )
endif(KDEVPG_FOUND)

kde4_automoc(${kdevqmakeparser_SRCS})
kde4_add_library(kdevqmakeparser SHARED ${kdevqmakeparser_SRCS})
target_link_libraries(kdevqmakeparser ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})
set_target_properties(kdevqmakeparser PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS kdevqmakeparser DESTINATION ${LIB_INSTALL_DIR} )

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdevqmakeparser)
#install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})

add_custom_target( copy-generated
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_ast.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmake_default_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_ast.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
    )
