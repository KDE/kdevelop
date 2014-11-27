add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentswitcher\")
project(documentswitcher)

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

add_library(kdevdocumentswitcher MODULE ${kdevdocumentswitcher_PART_SRCS})
target_link_libraries(kdevdocumentswitcher KDev::Interfaces KDev::Sublime KDev::Interfaces )

install(TARGETS kdevdocumentswitcher DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevdocumentswitcher.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentswitcher.desktop)
kcoreaddons_desktop_to_json(kdevdocumentswitcher ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentswitcher.desktop)
install( FILES kdevdocumentswitcher.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevdocumentswitcher )


