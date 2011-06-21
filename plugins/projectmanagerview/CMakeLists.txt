
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9511 )

########### next target ###############

set(kdevprojectmanagerview_PLUGIN_SRCS
    projectmanagerviewplugin.cpp
    projectmanagerview.cpp
    projecttreeview.cpp
    projectbuildsetwidget.cpp
    builditembuilderjob.cpp
    projectdashboardwidget.cpp
)

kde4_add_ui_files( kdevprojectmanagerview_PLUGIN_SRCS projectbuildsetwidget.ui projectmanagerview.ui )

kde4_add_plugin(kdevprojectmanagerview ${kdevprojectmanagerview_PLUGIN_SRCS})
target_link_libraries(kdevprojectmanagerview ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
                      kdevplatforminterfaces kdevplatformproject kdevplatformlanguage kdevplatformutil sublime ${KDE4_THREADWEAVER_LIBRARIES} )

install(TARGETS kdevprojectmanagerview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevdashboardproject.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanagerview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanagerview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanagerview )


