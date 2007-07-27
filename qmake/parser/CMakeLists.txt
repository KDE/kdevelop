
set(parser_STAT_SRCS
    qmakeast.cpp
    qmakeastassignment.cpp
    qmakeastbody.cpp
    qmakeastor.cpp
    qmakeastproject.cpp
    qmakeastfunc.cpp
    qmakeastscope.cpp
    qmakeastsimple.cpp
    qmakedriver.cpp
    qmakedebugvisitor.cpp
    buildastvisitor.cpp
    qmakelexer.cpp
    )

if(KDEVPG_FOUND)
    kdevpg_generate(parser_GEN_SRCS qmake NAMESPACE QMake DUMP_INFO
        "${CMAKE_SOURCE_DIR}/parser/qmake.g"
	    "${CMAKE_SOURCE_DIR}/parser/qmakelexer.h")

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDEVPG_INCLUDE_DIR} )

else(KDEVPG_FOUND)
    message(STATUS "Assuming existence of the generated parser files for the parser")
    set(parser_GEN_SRCS
        generated/qmake_parser.cpp
        generated/qmake_default_visitor.cpp
        generated/qmake_visitor.cpp)
    include_directories( generated
        generated/kdevelop-pg )
endif(KDEVPG_FOUND)

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdevqmakeparser )

kde4_add_library( kdevqmakeparser SHARED ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(kdevqmakeparser ${KDE4_KDECORE_LIBS})

install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})

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

