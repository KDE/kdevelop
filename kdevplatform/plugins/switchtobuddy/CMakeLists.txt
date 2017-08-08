add_definitions(-DTRANSLATION_DOMAIN=\"kdevswitchtobuddy\")
project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

ecm_qt_declare_logging_category(kdevswitchtobuddy_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_SWITCHTOBUDDY
    CATEGORY_NAME "kdevplatform.plugins.switchtobuddy"
)
qt5_add_resources(kdevswitchtobuddy_PART_SRCS kdevswitchtobuddy.qrc)
kdevplatform_add_plugin(kdevswitchtobuddy JSON kdevswitchtobuddy.json SOURCES ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    KDev::Interfaces
    KDev::Language
)
