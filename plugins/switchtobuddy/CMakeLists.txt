project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

add_library(kdevswitchtobuddy MODULE ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevswitchtobuddy DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevswitchtobuddy.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevswitchtobuddy.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevswitchtobuddy.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
