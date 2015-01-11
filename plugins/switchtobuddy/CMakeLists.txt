add_definitions(-DTRANSLATION_DOMAIN=\"kdevswitchtobuddy\")
project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

kdevplatform_add_plugin(kdevswitchtobuddy JSON kdevswitchtobuddy.json SOURCES ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    KDev::Interfaces
    KDev::Language
)

install( FILES kdevswitchtobuddy.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevswitchtobuddy)
