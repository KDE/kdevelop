add_definitions(-DTRANSLATION_DOMAIN=\"kdevswitchtobuddy\")
project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

qt5_add_resources(kdevswitchtobuddy_PART_SRCS kdevswitchtobuddy.qrc)
kdevplatform_add_plugin(kdevswitchtobuddy JSON kdevswitchtobuddy.json SOURCES ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    KDev::Interfaces
    KDev::Language
)
