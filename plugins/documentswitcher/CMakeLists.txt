project(documentswitcher)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9530 )

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

add_library(kdevdocumentswitcher MODULE ${kdevdocumentswitcher_PART_SRCS})
target_link_libraries(kdevdocumentswitcher KDev::Interfaces KDev::Sublime KDev::Interfaces )

install(TARGETS kdevdocumentswitcher DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevdocumentswitcher.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentswitcher.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentswitcher.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevdocumentswitcher.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevdocumentswitcher )


