
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9501 )
add_subdirectory(settings)
add_subdirectory(tests)

set(kdevplatformshell_LIB_SRCS
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
    documentationview.cpp
)

set(kdevplatformshell_UI
    sessiondialog.ui
    projectinfopage.ui
)

kde4_add_ui_files( kdevplatformshell_LIB_SRCS ${kdevplatformshell_UI} )
kde4_add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell 
        ${KDE4_KIO_LIBS} 
        ${KDE4_KFILE_LIBS}
        ${KDE4_KUTILS_LIBS} 
        ${KDE4_KPARTS_LIBS} 
        ${KDE4_KNOTIFYCONFIG_LIBS} 
        ${KDE4_KTEXTEDITOR_LIBS} 
        ${KDE4_THREADWEAVER_LIBRARIES} 
        ${QT_QTDESIGNER_LIBRARY} 
        kdevplatforminterfaces 
        kdevplatformproject 
        kdevplatformlanguage 
        sublime 
        kdevplatformutil 
        kdevplatformoutputview)
# need to add kdevplatform* once those are exported targets
target_link_libraries(kdevplatformshell LINK_INTERFACE_LIBRARIES 
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDECORE_LIBS})
set_target_properties(kdevplatformshell PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

set(kdevplatformtestshell_LIB_SRCS
    testcore.cpp
)

kde4_add_library(kdevplatformtestshell SHARED ${kdevplatformtestshell_LIB_SRCS})
target_link_libraries(kdevplatformtestshell kdevplatformshell ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformproject kdevplatformlanguage sublime kdevplatformutil kdevplatformoutputview)
set_target_properties(kdevplatformtestshell PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

install(TARGETS kdevplatformtestshell kdevplatformshell EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

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
    testshellexport.h
    testcore.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/shell COMPONENT Devel
)
