
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
    language.cpp
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
    settings/ccpreferences.cpp
    settings/bgpreferences.cpp

)

kconfig_add_kcfg_files(KDevPlatformShell_LIB_SRCS
    settings/uiconfig.kcfgc
    settings/projectconfig.kcfgc
    settings/ccconfig.kcfgc
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
    settings/projectsettings.ui
    settings/environmentwidget.ui
    settings/ccsettings.ui
    settings/bgsettings.ui
)

add_library(KDevPlatformShell ${KDevPlatformShell_LIB_SRCS})
add_library(KDev::Shell ALIAS KDevPlatformShell)

generate_export_header(KDevPlatformShell EXPORT_FILE_NAME shellexport.h)
target_link_libraries(KDevPlatformShell
LINK_PUBLIC
    KF5::XmlGui

    KDev::Sublime
    KDev::OutputView
    KDev::Debugger
    KDev::Interfaces
LINK_PRIVATE
    Qt5::Declarative

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

    KDev::Project
    KDev::Vcs
    KDev::Language
    KDev::Util
    KDev::Documentation
)
set_target_properties(KDevPlatformShell PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION} EXPORT_NAME Shell)

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
    language.h
    selectioncontroller.h
    runcontroller.h
    launchconfiguration.h
    ${CMAKE_CURRENT_BINARY_DIR}/shellexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/shell COMPONENT Devel
)

install(TARGETS KDevPlatformShell EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES debugger/kdevdebuggershellui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevdebugger )
install( FILES kdevsessionui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevsession )
install( FILES kdevsourceformatter.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevsourceformatter )
install( FILES AssistantButton.qml assistantpopup.qml DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
