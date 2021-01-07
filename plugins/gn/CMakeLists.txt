add_definitions(-DTRANSLATION_DOMAIN=\"kdevgnmanager\")

find_package(GN 0.51 QUIET)
set_package_properties(GN PROPERTIES
    DESCRIPTION "The GN build system"
    URL "https://gn.googlesource.com/gn/"
    PURPOSE "Recommended: required by the GN project manager plugin"
    TYPE RUNTIME
)

set(gnbuilder_SRCS
    gnbuilder.cpp
    gnconfig.cpp
    gnjob.cpp
    gnjobprune.cpp
    gnmanager.cpp
    gnimportprojectjob.cpp

    settings/gnadvancedsettings.cpp
    settings/gnconfigpage.cpp
    settings/gnnewbuilddir.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ki18n_wrap_ui(gnbuilder_SRCS
              settings/gnadvancedsettings.ui
              settings/gnconfigpage.ui
              settings/gnnewbuilddir.ui
)
ecm_qt_declare_logging_category(gnbuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_GN
    CATEGORY_NAME "kdevelop.plugins.gn"
)

kdevplatform_add_plugin(kdevgnmanager
                        JSON kdevgnmanager.json
                        SOURCES ${gnbuilder_SRCS})

target_link_libraries(kdevgnmanager
    Qt5::Concurrent
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KDev::OutputView
)
