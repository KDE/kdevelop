add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9010 )


########### next target ###############

set(kdevappwizard_PART_SRCS
    appwizardplugin.cpp
    appwizarddialog.cpp
    appwizardpagewidget.cpp
    projectselectionpage.cpp
    projecttemplatesmodel.cpp
    projectvcspage.cpp
)

set(kdevappwizard_PART_UI
    projectselectionpage.ui
    projectvcspage.ui
)

kde4_add_ui_files(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})

kde4_add_plugin(kdevappwizard ${kdevappwizard_PART_SRCS})
target_link_libraries(kdevappwizard
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces kdevplatformvcs)

install(TARGETS kdevappwizard DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevappwizard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevappwizard.rc DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
install( FILES default-kdevelop.png DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard/template_previews )
