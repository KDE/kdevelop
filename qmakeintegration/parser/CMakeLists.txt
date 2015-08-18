set(kdevqmakeparser_SRCS
    qmakeast.cpp
    qmakedriver.cpp
    qmakedebugvisitor.cpp
    buildastvisitor.cpp
    qmakelexer.cpp
    qmakeastvisitor.cpp
    qmakeastdefaultvisitor.cpp
    )

include_directories( ${QMakeManager_SOURCE_DIR}/parser ${QMakeManager_BINARY_DIR}/parser )

kdevpgqt_generate(parser_GEN_SRCS qmake NAMESPACE QMake DUMP_INFO
    "${QMakeManager_SOURCE_DIR}/parser/qmake.g"
    "${QMakeManager_SOURCE_DIR}/parser/qmakelexer.h")

include_directories(
    ${KDEVPGQT_INCLUDE_DIR} )

add_subdirectory(tests)

option(BUILD_qmake_parser "Build the qmake-parser debugging tool" OFF)
if(BUILD_qmake_parser)
  add_executable(qmake-parser ../debug.cpp main.cpp)
  target_link_libraries(qmake-parser kdevqmakeparser KF5::CoreAddons Qt5::Core)
  install(TARGETS qmake-parser ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

add_library(kdevqmakeparser STATIC ${kdevqmakeparser_SRCS} ${parser_GEN_SRCS})
generate_export_header(kdevqmakeparser BASE_NAME parser EXPORT_MACRO_NAME KDEVQMAKEPARSER_EXPORT)
target_link_libraries(kdevqmakeparser LINK_PRIVATE
    KF5::CoreAddons
    KF5::KIOCore
)
target_include_directories(kdevqmakeparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
