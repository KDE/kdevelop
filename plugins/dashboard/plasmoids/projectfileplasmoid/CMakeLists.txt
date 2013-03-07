project(projectitemelement)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(projectfileelement_SRCS projectfileelement.cpp)

kde4_add_ui_files(projectfileelement_SRCS config.ui)
kde4_add_plugin(plasma_kdev_projectfileelement ${projectfileelement_SRCS})
target_link_libraries(plasma_kdev_projectfileelement ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_kdev_projectfileelement DESTINATION ${PLUGIN_INSTALL_DIR})
configure_file(kdev-dash-projectfileelement.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdev-dash-projectfileelement.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdev-dash-projectfileelement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
