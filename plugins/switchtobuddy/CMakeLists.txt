add_definitions(-DTRANSLATION_DOMAIN=\"kdevswitchtobuddy\")

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

declare_qt_logging_category(kdevswitchtobuddy_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_SWITCHTOBUDDY
    CATEGORY_BASENAME "switchtobuddy"
)
qt_add_resources(kdevswitchtobuddy_PART_SRCS kdevswitchtobuddy.qrc)
kdevplatform_add_plugin(kdevswitchtobuddy SOURCES ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    KDev::Interfaces
    KDev::Language
)
