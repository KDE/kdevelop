include_directories(
    ${KDEVPGQT_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
set(kdevqmakeparser_SRCS
    qmakeast.cpp
    qmakedriver.cpp
    qmakedebugvisitor.cpp
    buildastvisitor.cpp
    qmakelexer.cpp
    qmakeastvisitor.cpp
    qmakeastdefaultvisitor.cpp
    )

kdevpgqt_generate(parser_GEN_SRCS qmake NAMESPACE QMake DUMP_INFO
    "${CMAKE_CURRENT_SOURCE_DIR}/qmake.g"
    "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.h")

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

option(BUILD_qmake_parser "Build the qmake-parser debugging tool" OFF)
if(BUILD_qmake_parser)
  add_executable(qmake-parser ${qmakecommon_LOG_SRCS} main.cpp)
  target_link_libraries(qmake-parser kdevqmakeparser KF5::CoreAddons Qt5::Core)
  install(TARGETS qmake-parser ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

add_library(kdevqmakeparser STATIC ${kdevqmakeparser_SRCS} ${parser_GEN_SRCS})
generate_export_header(kdevqmakeparser BASE_NAME parser EXPORT_MACRO_NAME KDEVQMAKEPARSER_EXPORT)
target_link_libraries(kdevqmakeparser
    PUBLIC
        KDev::Util
    PRIVATE
        KF5::CoreAddons
        KF5::KIOCore
)
target_include_directories(kdevqmakeparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
