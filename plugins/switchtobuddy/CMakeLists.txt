project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

add_library(kdevswitchtobuddy MODULE ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    KDev::Interfaces
    KDev::Language
)

install(TARGETS kdevswitchtobuddy DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevswitchtobuddy.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevswitchtobuddy.desktop)
kcoreaddons_desktop_to_json(kdevswitchtobuddy ${CMAKE_CURRENT_BINARY_DIR}/kdevswitchtobuddy.desktop)
