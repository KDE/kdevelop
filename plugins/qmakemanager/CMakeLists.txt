add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakemanager\")
remove_definitions(
    -DQT_NO_FOREACH
)

declare_qt_logging_category(qmakecommon_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_QMAKE
    CATEGORY_BASENAME "qmake"
)

add_subdirectory(parser)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(qmakecommon_SRCS
    qmakebuilddirchooser.cpp
    qmakebuilddirchooserdialog.cpp
    qmakecache.cpp
    qmakeconfig.cpp
    qmakefile.cpp
    qmakefilevisitor.cpp
    qmakeincludefile.cpp
    qmakemkspecs.cpp
    qmakeprojectfile.cpp
    qmakeutils.cpp
    variablereferenceparser.cpp
    ${qmakecommon_LOG_SRCS}
)
ki18n_wrap_ui(qmakecommon_SRCS qmakebuilddirchooser.ui)
add_library(kdevqmakecommon STATIC ${qmakecommon_SRCS})
target_link_libraries(kdevqmakecommon
    KDev::Interfaces KDev::Project KDev::Util
    kdevqmakeparser)

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakejob.cpp
)
kdevplatform_add_plugin(kdevqmakemanager JSON kdevqmakemanager.json SOURCES ${kdevqmakemanager_PART_SRCS})
target_link_libraries(kdevqmakemanager
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
    KDev::Serialization
    KDev::QMakeBuilder
    kdevqmakecommon
    kdevqmakeparser
)
