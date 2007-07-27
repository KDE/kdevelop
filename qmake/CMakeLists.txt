
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KDevPlatform REQUIRED)
find_package(KDevelop REQUIRED)
find_package(KDevelop-PG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES}
	${KDEVELOP_INCLUDE_DIR}/qmake/
	)

include_directories(${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    )

set(parser_STAT_SRCS
    ${CMAKE_SOURCE_DIR}/parser/qmakeast.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastassignment.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastbody.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastor.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastproject.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastfunc.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastscope.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakeastsimple.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakedriver.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakedebugvisitor.cpp
    ${CMAKE_SOURCE_DIR}/parser/buildastvisitor.cpp
    ${CMAKE_SOURCE_DIR}/parser/qmakelexer.cpp
    )

#find_package(KDevelop-PG QUIET)

macro(GET_GENERATED_SRCS _srcFiles)
    if(KDEVPG_FOUND)
        kdevpg_generate(_files qmake NAMESPACE QMake DUMP_INFO
            "${CMAKE_SOURCE_DIR}/parser/qmake.g"
    	    "${CMAKE_SOURCE_DIR}/parser/qmakelexer.h")

        include_directories(
            ${CMAKE_CURRENT_BINARY_DIR}
            ${KDEVPG_INCLUDE_DIR} )
        set(${_srcFiles} ${_files})

    else(KDEVPG_FOUND)
        message(STATUS "Assuming existence of the parser/generated parser files for the parser")
        set(${_srcFiles}
            ${CMAKE_SOURCE_DIR}/parser/generated/qmake_parser.cpp
            ${CMAKE_SOURCE_DIR}/parser/generated/qmake_default_visitor.cpp
            ${CMAKE_SOURCE_DIR}/parser/generated/qmake_visitor.cpp)
        include_directories( ${CMAKE_SOURCE_DIR}/parser/generated
            ${CMAKE_SOURCE_DIR}/parser/generated/kdevelop-pg )
    endif(KDEVPG_FOUND)
endmacro(GET_GENERATED_SRCS)

add_subdirectory(tests)

########### next target ###############

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectscope.cpp
    qmakemkspecs.cpp
    qmakefile.cpp
)


GET_GENERATED_SRCS(parser_GEN_SRCS)

kde4_add_plugin(kdevqmakemanager ${kdevqmakemanager_PART_SRCS} ${parser_GEN_SRCS} ${parser_STAT_SRCS})
target_link_libraries( kdevqmakemanager ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} )

add_executable(qmake-parser parser/main.cpp ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(qmake-parser ${KDE4_KDECORE_LIBS} )

########### install files ###############

install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})
install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

add_custom_target( copy-generated
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_ast.h" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_ast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_parser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_parser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_visitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_default_visitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_default_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_ast.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_default_visitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_visitor.h"
    )

