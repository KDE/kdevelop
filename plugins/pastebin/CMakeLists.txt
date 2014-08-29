set(kdevpastebin_PART_SRCS
    pastebinplugin.cpp
)

add_library(kdevpastebin MODULE ${kdevpastebin_PART_SRCS})
target_link_libraries(kdevpastebin KDev::Vcs)

install(TARGETS kdevpastebin DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevpastebin.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevpastebin.desktop)
kcoreaddons_desktop_to_json(kdevpastebin ${CMAKE_CURRENT_BINARY_DIR}/kdevpastebin.desktop)
