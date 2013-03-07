set(kdevpastebin_PART_SRCS
    pastebinplugin.cpp
)

kde4_add_plugin(kdevpastebin ${kdevpastebin_PART_SRCS})
target_link_libraries(kdevpastebin ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevpastebin DESTINATION ${PLUGIN_INSTALL_DIR})
configure_file(kdevpastebin.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevpastebin.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevpastebin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
