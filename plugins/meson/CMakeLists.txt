add_definitions(-DTRANSLATION_DOMAIN=\"kdevmesonmanager\")

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
ecm_qt_declare_logging_category(mesonbuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_Meson
    CATEGORY_NAME "kdevelop.plugins.meson"
)

kdevplatform_add_plugin(kdevmesonmanager
                        JSON kdevmesonmanager.json
                        SOURCES ${mesonbuilder_SRCS})

target_link_libraries(kdevmesonmanager
    Qt5::Concurrent
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KDev::OutputView
)

set_target_properties(kdevmesonmanager PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)
