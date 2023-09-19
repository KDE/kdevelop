add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakemanager\")

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
    KF${KF_MAJOR_VERSION}::KIOWidgets
    kdevqmakeparser)

kdevplatform_add_plugin(KDevQMakeManager SOURCES qmakemanager.cpp
    qmakemodelitems.cpp
    qmakejob.cpp)
target_link_libraries(KDevQMakeManager
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
    KDev::Serialization
    KDev::QMakeBuilder
    kdevqmakecommon
    kdevqmakeparser
)
