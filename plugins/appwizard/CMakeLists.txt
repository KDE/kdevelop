include_directories( ${KDEVPLATFORM_INCLUDE_DIR} ${KDEVPLATFORM_INCLUDE_DIR}/interfaces )


########### next target ###############

set(kdevappwizard_PART_SRCS
    appwizardplugin.cpp
    appwizarddialog.cpp
    projectselectionpage.cpp
    projecttemplatesmodel.cpp
    projecttemplateitem.cpp
    importproject.cpp
    projectvcspage.cpp
)

set(kdevappwizard_PART_UI
    projectselectionpage.ui
    importproject.ui
    projectvcspage.ui
)

kde4_add_ui_files(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})

kde4_add_plugin(kdevappwizard ${kdevappwizard_PART_SRCS})
target_link_libraries(kdevappwizard
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_VCS_LIBRARY})

install(TARGETS kdevappwizard DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevappwizard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevappwizard.rc DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
