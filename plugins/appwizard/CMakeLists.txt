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

declare_qt_logging_category(kdevappwizard_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_APPWIZARD
    CATEGORY_BASENAME "appwizard"
)

ki18n_wrap_ui(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})

if(NOT KF5_VERSION VERSION_LESS "5.57.0")
install(FILES kdevappwizard.knsrc  DESTINATION ${KDE_INSTALL_KNSRCDIR})
else()
qt5_add_resources(kdevappwizard_PART_SRCS kdevappwizardknsrc.qrc)
endif()
qt5_add_resources(kdevappwizard_PART_SRCS kdevappwizard.qrc)
kdevplatform_add_plugin(kdevappwizard JSON kdevappwizard.json SOURCES ${kdevappwizard_PART_SRCS})

target_link_libraries(kdevappwizard
    KDev::Interfaces
    KDev::Vcs
    KDev::Language
    KDev::Sublime
    KDev::Util
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::Archive
)


