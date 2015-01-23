include_directories( ${QMakeManager_BINARY_DIR}/parser ${QMakeManager_SOURCE_DIR}/parser )

set(duchain_STAT_SRCS
    contextbuilder.cpp
    qmakeeditorintegrator.cpp
)
add_library(kdevqmakeduchain STATIC ${duchain_STAT_SRCS})
target_link_libraries(kdevqmakeduchain KF5::KDELibs4Support kdevqmakeparser KDev::Language KF5::TextEditor)
