##############################
# PLUGIN #####################
##############################

set( projectfilter_SRCS
    projectfilterprovider.cpp
    projectfilter.cpp
    projectfilterdebug.cpp
    filter.cpp
)

kde4_add_plugin( kdevprojectfilter ${projectfilter_SRCS} )
target_link_libraries( kdevprojectfilter
    ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS}
    kdevplatformproject kdevplatformutil kdevplatforminterfaces)

install( TARGETS kdevprojectfilter DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevprojectfilter.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

##############################
# KCM ########################
##############################

set( projectfilterkcm_SRCS
    projectfilterkcm.cpp
    projectfilterdebug.cpp
    filter.cpp
    filtermodel.cpp
    comboboxdelegate.cpp
)

set( projectfilterkcm_UI
    projectfiltersettings.ui
)

kde4_add_ui_files( projectfilterkcm_SRCS ${projectfilterkcm_UI} )
kde4_add_kcfg_files( projectfilterkcm_SRCS projectfiltersettings.kcfgc )
kde4_add_plugin( kcm_kdevprojectfilter ${projectfilterkcm_SRCS} )
target_link_libraries( kcm_kdevprojectfilter
    ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS}
    kdevplatformproject kdevplatformutil kdevplatforminterfaces)

install( TARGETS kcm_kdevprojectfilter DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdevprojectfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(tests)
