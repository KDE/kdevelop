add_definitions(-DTRANSLATION_DOMAIN=\"kdevkonsole\")
########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleviewplugin.cpp
    kdevkonsoleview.cpp
)

declare_qt_logging_category(kdevkonsoleview_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_KONSOLE
    CATEGORY_BASENAME "konsole"
)
kdevplatform_add_plugin(kdevkonsoleview JSON kdevkonsoleview.json SOURCES ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF5::I18n KF5::Parts KDev::Interfaces KDev::Util)
