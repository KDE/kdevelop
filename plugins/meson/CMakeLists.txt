add_definitions(-DTRANSLATION_DOMAIN=\"kdevmesonmanager\")

find_package(Meson 0.51 QUIET)
set_package_properties(Meson PROPERTIES
    DESCRIPTION "The Meson build system"
    URL "https://mesonbuild.com"
    PURPOSE "Recommended: required by the Meson project manager plugin"
    TYPE RUNTIME
)

set(mesonbuilder_SRCS
    mesonbuilder.cpp
    mesonconfig.cpp
    mesonjob.cpp
    mesonjobprune.cpp
    mesonmanager.cpp

    mintro/mesonintrospectjob.cpp
    mintro/mesonoptions.cpp
    mintro/mesonprojectinfo.cpp
    mintro/mesontargets.cpp
    mintro/mesontests.cpp

    rewriter/mesonactionbase.cpp
    rewriter/mesondefaultopts.cpp
    rewriter/mesonkwargsinfo.cpp
    rewriter/mesonkwargsmodify.cpp
    rewriter/mesonrewriterjob.cpp

    settings/mesonadvancedsettings.cpp
    settings/mesonconfigpage.cpp
    settings/mesonlisteditor.cpp
    settings/mesonnewbuilddir.cpp
    settings/mesonoptionbaseview.cpp
    settings/mesonoptionsview.cpp
    settings/mesonrewriterinput.cpp
    settings/mesonrewriterpage.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ki18n_wrap_ui(mesonbuilder_SRCS
              settings/mesonadvancedsettings.ui
              settings/mesonconfigpage.ui
              settings/mesonlisteditor.ui
              settings/mesonnewbuilddir.ui
              settings/mesonoptionbaseview.ui
              settings/mesonoptionsview.ui
              settings/mesonrewriterinput.ui
              settings/mesonrewriteroptioncontainer.ui
              settings/mesonrewriterpage.ui
)
declare_qt_logging_category(mesonbuilder_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_Meson
    CATEGORY_BASENAME "meson"
)

kdevplatform_add_plugin(KDevMesonManager
                        SOURCES ${mesonbuilder_SRCS})

target_link_libraries(KDevMesonManager
    Qt::Concurrent
    KF6::KIOWidgets
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KDev::OutputView
)
