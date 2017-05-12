add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakemanager\")

ecm_qt_declare_logging_category(qmakecommon_LOG_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_QMAKE
    CATEGORY_NAME "kdevelop.projectmanagers.qmake"
)

add_subdirectory(parser)
add_subdirectory(tests)

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
    kdevqmakecommon
    kdevqmakeparser)
