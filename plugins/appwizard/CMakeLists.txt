add_definitions(-DTRANSLATION_DOMAIN=\"kdevappwizard\")

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

kdevplatform_add_plugin(kdevappwizard JSON kdevappwizard.json SOURCES ${kdevappwizard_PART_SRCS})

target_link_libraries(kdevappwizard
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::Archive
    KDev::Interfaces KDev::Vcs KDev::Language KDev::Util)

install(TARGETS kdevappwizard DESTINATION ${KDE_INSTALL_PLUGINDIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

install( FILES kdevappwizard.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kdevappwizard )
install( FILES kdevappwizard.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )
