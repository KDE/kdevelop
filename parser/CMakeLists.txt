
set(parser_STAT_SRCS
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
  add_executable(qmake-parser main.cpp)
  target_link_libraries(qmake-parser kdevqmakeparser Qt5::Core  )
  install(TARGETS qmake-parser ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_qmake_parser)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
add_library( kdevqmakeparser SHARED ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(kdevqmakeparser LINK_PRIVATE
    KF5::KDELibs4Support
)

install(TARGETS kdevqmakeparser ${INSTALL_TARGETS_DEFAULT_ARGS})

