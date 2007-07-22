
find_package(KDE4 REQUIRED)
find_package(KDevelop-PG REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEVPG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(parser_STAT_SRCS
    ${CMAKE_SOURCE_DIR}/qmakedriver.cpp
    ${CMAKE_SOURCE_DIR}/qmakedebugvisitor.cpp
    ${CMAKE_SOURCE_DIR}/qmakelexer.cpp
    ${CMAKE_SOURCE_DIR}/qmakeast.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastassignment.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastbody.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastfunc.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastor.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastproject.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastscope.cpp
    ${CMAKE_SOURCE_DIR}/qmakeastsimple.cpp
    ${CMAKE_SOURCE_DIR}/buildastvisitor.cpp
    )

kdevpg_generate(_files qmake NAMESPACE QMake DUMP_INFO
      "${CMAKE_SOURCE_DIR}/qmake.g"
      "${CMAKE_SOURCE_DIR}/qmakelexer.h")

set(parser_GEN_SRCS ${_files})

add_executable(qmake-parser main.cpp ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(qmake-parser ${KDE4_KDECORE_LIBS} )

install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})

