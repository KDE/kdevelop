add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakemanager\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(parser)
add_subdirectory(tests)

set(qmakecommon_SRCS
    debug.cpp
    qmakebuilddirchooser.cpp
    qmakebuilddirchooserdialog.cpp
    qmakeconfig.cpp
)
ki18n_wrap_ui(qmakecommon_SRCS qmakebuilddirchooser.ui)
add_library(kdevqmakecommon STATIC ${qmakecommon_SRCS})
target_link_libraries(kdevqmakecommon
    KDev::Interfaces KDev::Project KDev::Util KDev::Language
    kdevqmakeparser)

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
)
kdevplatform_add_plugin(kdevqmakemanager JSON kdevqmakemanager.json SOURCES ${kdevqmakemanager_PART_SRCS})
target_link_libraries(kdevqmakemanager
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
    KDev::Serialization
    kdevqmakecommon
    kdevqmakeparser)
