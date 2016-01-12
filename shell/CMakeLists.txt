add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

add_subdirectory(tests)

set(KDevPlatformShell_LIB_SRCS
    workingsetcontroller.cpp
    workingsets/workingset.cpp
    workingsets/workingsetfilelabel.cpp
    workingsets/workingsettoolbutton.cpp
    workingsets/workingsettooltipwidget.cpp
    workingsets/workingsetwidget.cpp
    workingsets/closedworkingsetswidget.cpp
    workingsets/workingsethelpers.cpp
    assistantpopup.cpp
    mainwindow.cpp
    mainwindow_p.cpp
    plugincontroller.cpp
    ktexteditorpluginintegration.cpp
    shellextension.cpp
    core.cpp
    uicontroller.cpp
    projectcontroller.cpp
    project.cpp
    partcontroller.cpp
    #document.cpp
    partdocument.cpp
    textdocument.cpp
    documentcontroller.cpp
    languagecontroller.cpp
    statusbar.cpp
    runcontroller.cpp
    sessioncontroller.cpp
    session.cpp
    sessionlock.cpp
    sessionchooserdialog.cpp
    savedialog.cpp
    sessiondialog.cpp
    sourceformattercontroller.cpp
    completionsettings.cpp
    openprojectpage.cpp
    openprojectdialog.cpp
    projectinfopage.cpp
    selectioncontroller.cpp
    documentationcontroller.cpp
    debugcontroller.cpp
    launchconfiguration.cpp
    launchconfigurationdialog.cpp
    loadedpluginsdialog.cpp
    testcontroller.cpp
    projectsourcepage.cpp
    debug.cpp
    configdialog.cpp
    editorconfigpage.cpp
    environmentconfigurebutton.cpp

    checkerstatus.cpp
    problem.cpp
    problemmodelset.cpp
    problemmodel.cpp
    problemstore.cpp
    watcheddocumentset.cpp
    filteredproblemstore.cpp

    progresswidget/progressmanager.cpp
    progresswidget/statusbarprogresswidget.cpp
    progresswidget/overlaywidget.cpp
    progresswidget/progressdialog.cpp

    areadisplay.cpp

    settings/uipreferences.cpp
    settings/pluginpreferences.cpp
    settings/sourceformattersettings.cpp
    settings/editstyledialog.cpp
    settings/projectpreferences.cpp
    settings/environmentwidget.cpp
    settings/environmentgroupmodel.cpp
    settings/environmentpreferences.cpp
    settings/languagepreferences.cpp
    settings/bgpreferences.cpp
    settings/templateconfig.cpp
    settings/templatepage.cpp

)

kconfig_add_kcfg_files(KDevPlatformShell_LIB_SRCS
    settings/uiconfig.kcfgc
    settings/projectconfig.kcfgc
    settings/languageconfig.kcfgc
    settings/bgconfig.kcfgc
)

ki18n_wrap_ui(KDevPlatformShell_LIB_SRCS
    sessiondialog.ui
    projectinfopage.ui
    launchconfigurationdialog.ui
    projectsourcepage.ui
    settings/uiconfig.ui
    settings/editstyledialog.ui
    settings/sourceformattersettings.ui
    settings/projectpreferences.ui
    settings/environmentwidget.ui
    settings/languagepreferences.ui
    settings/bgpreferences.ui
    settings/templateconfig.ui
    settings/templatepage.ui
)

kdevplatform_add_library(KDevPlatformShell SOURCES ${KDevPlatformShell_LIB_SRCS})
target_link_libraries(KDevPlatformShell
LINK_PUBLIC
    KF5::XmlGui

    KDev::Sublime
    KDev::OutputView
    KDev::Interfaces
LINK_PRIVATE
    Qt5::Quick
    Qt5::QuickWidgets

    KF5::GuiAddons
    KF5::IconThemes
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::Parts
    KF5::Notifications
    KF5::NotifyConfig
    KF5::TextEditor
    KF5::ThreadWeaver
    KF5::JobWidgets
    KF5::ItemViews
    KF5::WindowSystem
    KF5::KCMUtils #for KPluginSelector, not sure why it is in kcmutils
    KF5::NewStuff # template config page
    KF5::Archive # template config page

    KDev::Debugger
    KDev::Project
    KDev::Vcs
    KDev::Language
    KDev::Util
    KDev::Documentation
)

install(FILES
    mainwindow.h
    plugincontroller.h
    shellextension.h
    core.h
    uicontroller.h
    projectcontroller.h
    project.h
    partcontroller.h
    partdocument.h
    textdocument.h
    documentcontroller.h
    languagecontroller.h
    session.h
    sessioncontroller.h
    sessionlock.h
    sourceformattercontroller.h
    selectioncontroller.h
    runcontroller.h
    launchconfiguration.h
    environmentconfigurebutton.h
    checkerstatus.h
    problem.h
    problemmodel.h
    problemmodelset.h
    problemconstants.h
    filteredproblemstore.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/shell COMPONENT Devel
)

install( FILES debugger/kdevdebuggershellui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kdevdebugger )
install( FILES kdevsessionui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kdevsession )
install( FILES kdevsourceformatter.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kdevsourceformatter )
install( FILES AssistantButton.qml assistantpopup.qml DESTINATION ${KDE_INSTALL_DATADIR}/kdevelop )
