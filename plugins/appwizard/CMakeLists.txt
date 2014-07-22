add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9010 )


########### next target ###############

set(kdevappwizard_PART_SRCS
    appwizardplugin.cpp
    appwizarddialog.cpp
    appwizardpagewidget.cpp
    projectselectionpage.cpp
#     projecttemplatesmodel.cpp
    projectvcspage.cpp
)

set(kdevappwizard_PART_UI
    projectselectionpage.ui
    projectvcspage.ui
)

qt5_wrap_ui(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})

add_library(kdevappwizard MODULE ${kdevappwizard_PART_SRCS})
target_link_libraries(kdevappwizard
    
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::Archive
    kdevplatforminterfaces kdevplatformvcs kdevplatformlanguage kdevplatformutil)

install(TARGETS kdevappwizard DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevappwizard.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevappwizard.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevappwizard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevappwizard.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevappwizard )
install( FILES kdevappwizard.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
