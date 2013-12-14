
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
  target_link_libraries(qmake-parser kdev4qmakeparser ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBRARY} )
  install(TARGETS qmake-parser ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_qmake_parser)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library( kdev4qmakeparser SHARED ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(kdev4qmakeparser LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
)

install(TARGETS kdev4qmakeparser ${INSTALL_TARGETS_DEFAULT_ARGS})

