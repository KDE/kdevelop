
include_directories( ${QMakeManager_BINARY_DIR}/parser ${QMakeManager_SOURCE_DIR}/parser )

set(duchain_STAT_SRCS
    contextbuilder.cpp
    qmakeeditorintegrator.cpp
    )

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
add_library( kdev4qmakeduchain SHARED ${duchain_STAT_SRCS})
target_link_libraries(kdev4qmakeduchain KF5::KDELibs4Support kdev4qmakeparser KDev::Language KF5::TextEditor )

install(TARGETS kdev4qmakeduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

