set(kdevkdeprovider_PART_SRCS
    kdeproviderplugin.cpp
)

kde4_add_plugin(kdevkdeprovider ${kdevkdeprovider_PART_SRCS})
target_link_libraries(kdevkdeprovider kdevplatforminterfaces kdevplatformvcs)

install(TARGETS kdevkdeprovider DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkdeprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})
