
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

# TODO: Re-Enable after generating new kross bindings for project model
#     kross/krossplugin.cpp
#     kross/krossbuildsystemmanager.cpp
#     kross/krossdistributedversioncontrol.cpp
#     kross/krosstoolviewfactory.cpp
#     kross/krossvcsjob.cpp
#     kross/wrappers/krosscontext.cpp
#     kross/wrappers/krosscontextmenuextension.cpp
#     kross/wrappers/krossvcsrevision.cpp
#     kross/wrappers/krossvcslocation.cpp
#     kross/wrappers/krossprojectmodel.cpp
# #    kross/wrappers/krossiuicontroller.cpp
#     kross/wrappers/krossidocument.cpp
# #    kross/wrappers/krosstopducontext.cpp
# #    kross/wrappers/krossducontext.cpp
#     kross/wrappers/krossilanguage.cpp
# #     kross/wrappers/krossdeclaration.cpp
# #     kross/wrappers/krossuse.cpp
#     kross/wrappers/krossidentifier.cpp
#     kross/wrappers/krossduchainlock.cpp
#     kross/wrappers/krossiproblem.cpp
#     kross/wrappers/krossibuildsystemmanager.cpp
#     kross/wrappers/krossiprojectfilemanager.cpp
#     kross/wrappers/krossiprojectbuilder.cpp
#     kross/wrappers/krosseditorcontext.cpp
#     kross/wrappers/krossqtoolbar.cpp
)

if(HAVE_KOMPARE)
	set(kdevplatformshell_LIB_SRCS ${kdevplatformshell_LIB_SRCS} patchdocument.cpp)
endif(HAVE_KOMPARE)

# Check if KTE is new enough to have the EditVIMode view mode flag
try_compile( HAVE_EDITVIMODE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/testeditvimode.cpp
             CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${KDE4_INCLUDES}"
             LINK_LIBRARIES ${KDE4_KTEXTEDITOR_LIBS} )
if(HAVE_EDITVIMODE)
    add_definitions("-DKTEXTEDITOR_HAS_VIMODE=1")
endif(HAVE_EDITVIMODE)

set(kdevplatformshell_UI
    sessiondialog.ui
    projectinfopage.ui
    launchconfigurationdialog.ui
    projectsourcepage.ui
)

kde4_add_ui_files( kdevplatformshell_LIB_SRCS ${kdevplatformshell_UI} )
kde4_add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_KCMUTILS_LIBS}
        ${KDE4_KPARTS_LIBS}
        ${KDE4_KNOTIFYCONFIG_LIBS}
        ${KDE4_KTEXTEDITOR_LIBS}
        ${KDE4_THREADWEAVER_LIBRARIES}
        ${KDE4_KROSSCORE_LIBS}
        kdevplatforminterfaces
        kdevplatformproject
        kdevplatformvcs
        kdevplatformlanguage
        sublime
        kdevplatformutil
        kdevplatformoutputview
        kdevplatformdebugger
        kdevplatformdocumentation)
# need to add kdevplatform* once those are exported targets
target_link_libraries(kdevplatformshell LINK_INTERFACE_LIBRARIES
        kdevplatformoutputview
        sublime
        kdevplatformdebugger)
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

if(NOT WIN32)
   macro_optional_add_subdirectory(kross)
endif(NOT WIN32)

