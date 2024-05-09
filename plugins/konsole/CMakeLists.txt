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
kdevplatform_add_plugin(kdevkonsoleview SOURCES ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF6::Parts KF6::Service KDev::Interfaces KDev::Util)
