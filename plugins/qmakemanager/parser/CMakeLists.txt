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

if(NOT KDevelop-PG-Qt_VERSION VERSION_LESS "2.2.1")
    set(_kdevpgqt_generate_extra_args ENFORCE_COMPATIBLE_ERROR_AWARE_CODE)
endif()
kdevpgqt_generate(parser_GEN_SRCS qmake NAMESPACE QMake DUMP_INFO ${_kdevpgqt_generate_extra_args}
    "${CMAKE_CURRENT_SOURCE_DIR}/qmake.g"
    "${CMAKE_CURRENT_SOURCE_DIR}/qmakelexer.h")

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

option(BUILD_qmake_parser "Build the qmake-parser debugging tool" OFF)
if(BUILD_qmake_parser)
  add_executable(qmake-parser ${qmakecommon_LOG_SRCS} main.cpp)
  target_link_libraries(qmake-parser kdevqmakeparser KF${KF_MAJOR_VERSION}::CoreAddons Qt${QT_MAJOR_VERSION}::Core)
  install(TARGETS qmake-parser ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

add_library(kdevqmakeparser STATIC ${kdevqmakeparser_SRCS} ${parser_GEN_SRCS})
generate_export_header(kdevqmakeparser BASE_NAME parser EXPORT_MACRO_NAME KDEVQMAKEPARSER_EXPORT)
target_link_libraries(kdevqmakeparser
    PUBLIC
        KDev::Util
    PRIVATE
        KF${KF_MAJOR_VERSION}::CoreAddons
        KF${KF_MAJOR_VERSION}::KIOCore
)
target_include_directories(kdevqmakeparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(QT_MAJOR_VERSION STREQUAL 6)
    target_link_libraries(kdevqmakeparser PRIVATE Qt6::Core5Compat)
endif()

# for proper include stack reports with gcc when working on the parser:
# remove ENFORCE_COMPATIBLE_ERROR_AWARE_CODE from kdevpgqt_generate
# uncomment next line:
# add_target_compile_flag_if_supported(kdevqmakeparser PRIVATE "-Wno-pedantic")
