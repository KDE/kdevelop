add_subdirectory(tests)

set(kdevkdeprovider_PART_SRCS
    kdeproviderwidget.cpp
    kdeproviderplugin.cpp
    kdeprojectsmodel.cpp
    kdeprojectsreader.cpp
)
kde4_add_kcfg_files(kdevkdeprovider_PART_SRCS kdeconfig.kcfgc)
kde4_add_ui_files(kdevkdeprovider_PART_SRCS kdeconfig.ui)

kde4_add_plugin(kdevkdeprovider ${kdevkdeprovider_PART_SRCS})
target_link_libraries(kdevkdeprovider ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_VCS_LIBRARIES})

install(TARGETS kdevkdeprovider DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkdeprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})
