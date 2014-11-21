

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

ki18n_wrap_ui(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})

add_library(kdevappwizard MODULE ${kdevappwizard_PART_SRCS})
target_link_libraries(kdevappwizard
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::Archive
    KF5::KDELibs4Support
    KDev::Interfaces KDev::Vcs KDev::Language KDev::Util)

install(TARGETS kdevappwizard DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevappwizard.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevappwizard.desktop)
kcoreaddons_desktop_to_json(kdevappwizard ${CMAKE_CURRENT_BINARY_DIR}/kdevappwizard.desktop)
install( FILES kdevappwizard.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevappwizard )
install( FILES kdevappwizard.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
