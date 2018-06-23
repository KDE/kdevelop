add_definitions(-DTRANSLATION_DOMAIN=\"kdevkonsole\")
########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleviewplugin.cpp
    kdevkonsoleview.cpp
)

ecm_qt_declare_logging_category(kdevkonsoleview_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_KONSOLE
    CATEGORY_NAME "kdevelop.plugins.konsole"
)
kdevplatform_add_plugin(kdevkonsoleview JSON kdevkonsoleview.json SOURCES ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF5::Parts KDev::Interfaces KDev::Util)
