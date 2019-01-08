add_definitions(-DTRANSLATION_DOMAIN=\"kdevmesonmanager\")

set(mesonbuilder_SRCS
    mesonbuilder.cpp
    mesonconfig.cpp
    mesonimportjob.cpp
    mesonjob.cpp
    mesonjobprune.cpp
    mesonmanager.cpp

    settings/mesonadvancedsettings.cpp
    settings/mesonconfigpage.cpp
    settings/mesonnewbuilddir.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ki18n_wrap_ui(mesonbuilder_SRCS settings/mesonadvancedsettings.ui settings/mesonconfigpage.ui settings/mesonnewbuilddir.ui)
ecm_qt_declare_logging_category(mesonbuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_Meson
    CATEGORY_NAME "kdevelop.plugins.meson"
)

kdevplatform_add_plugin(kdevmesonmanager
                        JSON kdevmesonmanager.json
                        SOURCES ${mesonbuilder_SRCS})

add_subdirectory(icons)

target_link_libraries(kdevmesonmanager
    Qt5::Concurrent
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
)
