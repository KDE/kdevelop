
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9501 )
add_subdirectory(settings)
add_subdirectory(tests)

set(kdevplatformshell_LIB_SRCS
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

    progresswidget/progressmanager.cpp
    progresswidget/statusbarprogresswidget.cpp
    progresswidget/overlaywidget.cpp
    progresswidget/progressdialog.cpp

    areadisplay.cpp
)

# Check if KTE is new enough to have the EditVIMode view mode flag
try_compile( HAVE_EDITVIMODE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/testeditvimode.cpp
             CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${KDE4_KTEXTEDITOR_INCLUDES}"
             LINK_LIBRARIES KF5::TextEditor )
if(HAVE_EDITVIMODE)
    add_definitions("-DKTEXTEDITOR_HAS_VIMODE=1")
endif()

set(kdevplatformshell_UI
    sessiondialog.ui
    projectinfopage.ui
    launchconfigurationdialog.ui
    projectsourcepage.ui
)

qt5_wrap_ui( kdevplatformshell_LIB_SRCS ${kdevplatformshell_UI} )
add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell LINK_PUBLIC
    KF5::XmlGui

    sublime
    kdevplatformoutputview
    kdevplatformdebugger
)
target_link_libraries(kdevplatformshell
LINK_PRIVATE
    KF5::KIOWidgets
    KF5::KCMUtils
    KF5::Parts
    KF5::NotifyConfig
    KF5::TextEditor
    KF5::ThreadWeaver
    KF5::JobWidgets
    KF5::ItemViews

    kdevplatformproject
    kdevplatformvcs
    kdevplatformlanguage
    kdevplatformutil
    kdevplatformdocumentation

LINK_PUBLIC
    kdevplatforminterfaces
    KF5::KDE4Support
)
set_target_properties(kdevplatformshell PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

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
    shellexport.h
    selectioncontroller.h
    runcontroller.h
    launchconfiguration.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/shell COMPONENT Devel
)

install(TARGETS kdevplatformshell EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES debugger/kdevdebuggershellui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevdebugger )
install( FILES kdevsessionui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsession )
install( FILES kdevsourceformatter.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsourceformatter )

