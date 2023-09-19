add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(KDevPlatformShell_LIB_SRCS
    mainwindow.cpp
    mainwindow_p.cpp
    plugincontroller.cpp
    ktexteditorpluginintegration.cpp
    shellextension.cpp
    core.cpp
    uicontroller.cpp
    colorschemechooser.cpp
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
    unitylauncher.cpp
    sessioncontroller.cpp
    session.cpp
    sessionlock.cpp
    sessionchooserdialog.cpp
    savedialog.cpp
    sourceformattercontroller.cpp
    sourceformatterjob.cpp
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
    configdialog.cpp
    editorconfigpage.cpp
    environmentconfigurebutton.cpp
    sourceformatterselectionedit.cpp
    runtimecontroller.cpp

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
    settings/environmentprofilemodel.cpp
    settings/environmentprofilelistmodel.cpp
    settings/environmentpreferences.cpp
    settings/languagepreferences.cpp
    settings/bgpreferences.cpp
    settings/templateconfig.cpp
    settings/templatepage.cpp
    settings/analyzerspreferences.cpp
    settings/runtimespreferences.cpp
    settings/documentationpreferences.cpp
)

if(APPLE)
    set(KDevPlatformShell_LIB_SRCS ${KDevPlatformShell_LIB_SRCS}
        macdockprogressview.mm
    )
endif()

declare_qt_logging_category(KDevPlatformShell_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "shell"
)

kconfig_add_kcfg_files(KDevPlatformShell_LIB_SRCS
    settings/uiconfig.kcfgc
    settings/projectconfig.kcfgc
    settings/languageconfig.kcfgc
    settings/bgconfig.kcfgc
)

ki18n_wrap_ui(KDevPlatformShell_LIB_SRCS
    projectinfopage.ui
    launchconfigurationdialog.ui
    projectsourcepage.ui
    sourceformatterselectionedit.ui
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

qt_add_resources(KDevPlatformShell_LIB_SRCS kdevplatformshell.qrc)


set(kdevworkingsets_SRCS
    workingsetcontroller.cpp
    workingsets/workingset.cpp
    workingsets/workingsetfilelabel.cpp
    workingsets/workingsettoolbutton.cpp
    workingsets/workingsettooltipwidget.cpp
    workingsets/workingsetwidget.cpp
    workingsets/closedworkingsetswidget.cpp
    workingsets/workingsethelpers.cpp
)

declare_qt_logging_category(kdevworkingsets_SRCS
    HEADER debug_workingset.h
    TYPE LIBRARY
    CATEGORY_BASENAME "workingset"
)

add_library(kdevworkingsets OBJECT
    ${kdevworkingsets_SRCS}
)

target_link_libraries(kdevworkingsets
PUBLIC
    KDev::Sublime
    KDev::Interfaces
PRIVATE
    KDev::Util
    KF6::TextEditor
)


kdevplatform_add_library(KDevPlatformShell SOURCES ${KDevPlatformShell_LIB_SRCS})
target_link_libraries(KDevPlatformShell
PUBLIC
    KDev::Sublime
    KDev::OutputView
    KDev::Interfaces
    KDev::Language
    KF6::XmlGui
PRIVATE
    KDev::Debugger
    KDev::Project
    KDev::Vcs
    KDev::Util
    KDev::Documentation
    KF6::GuiAddons
    KF6::ConfigWidgets
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    KF6::Parts
    KF6::Notifications
    KF6::NotifyConfig
    KF6::TextEditor
    KF6::JobWidgets
    KF6::ItemViews
    KF6::WindowSystem
    KF6::TextWidgets
    KF6::KCMUtils #for KPluginSelector, not sure why it is in kcmutils
    KF6::NewStuffWidgets # template config page
    KF6::Archive # template config page
    kdevworkingsets
)
if(APPLE)
    target_link_libraries(KDevPlatformShell PRIVATE "-framework AppKit")
endif()

install(FILES
    mainwindow.h
    plugincontroller.h
    shellextension.h
    core.h
    uicontroller.h
    colorschemechooser.h
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
    sourceformatterconfig.h
    sourceformattercontroller.h
    selectioncontroller.h
    runcontroller.h
    launchconfiguration.h
    environmentconfigurebutton.h
    sourceformatterselectionedit.h
    problem.h
    problemmodel.h
    problemmodelset.h
    problemconstants.h
    problemstore.h
    filteredproblemstore.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/shell COMPONENT Devel
)
