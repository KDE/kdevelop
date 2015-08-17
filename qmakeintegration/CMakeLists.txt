add_subdirectory(qmakebuilder)
add_subdirectory(app_templates)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# FIXME
remove_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x050400)

add_subdirectory(parser)
add_subdirectory(tests)

set(kdevqmakemanager_PART_SRCS
    debug.cpp
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectfile.cpp
    qmakeincludefile.cpp
    qmakemkspecs.cpp
    qmakefile.cpp
    qmakefilevisitor.cpp
    qmakecache.cpp
    variablereferenceparser.cpp
    qmakejob.cpp
    qmakebuilddirchooser.cpp
    qmakebuilddirchooserdialog.cpp
    qmakeconfig.cpp
)

set(kdevqmakemanager_PART_UI
    qmakebuilddirchooser.ui
)

ki18n_wrap_ui( kdevqmakemanager_PART_SRCS ${kdevqmakemanager_PART_UI} )

kdevplatform_add_plugin(kdevqmakemanager JSON kdevqmakemanager.json SOURCES ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
    KDev::Serialization
    kdevqmakeparser)
