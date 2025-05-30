add_definitions(-DTRANSLATION_DOMAIN=\"kdevappwizard\")

########### next target ###############

set(kdevappwizard_PART_SRCS
    appwizardplugin.cpp
    appwizarddialog.cpp
    appwizardpagewidget.cpp
    projectselectionpage.cpp
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

install(FILES kdevappwizard.knsrc  DESTINATION ${KDE_INSTALL_KNSRCDIR})
qt_add_resources(kdevappwizard_PART_SRCS kdevappwizard.qrc)
kdevplatform_add_plugin(kdevappwizard SOURCES ${kdevappwizard_PART_SRCS})

target_link_libraries(kdevappwizard
    KDev::Interfaces
    KDev::Vcs
    KDev::Language
    KDev::Sublime
    KDev::Util
    KF6::KIOWidgets
    KF6::NewStuffWidgets
    KF6::Archive
    Qt::Core5Compat
)
