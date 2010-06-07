
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9502 )
set(kdevplatforminterfaces_LIB_SRCS
    ipatchsource.cpp
    iassistant.cpp
    context.cpp
	ipatchdocument.cpp
    iplugin.cpp
    idocument.cpp
    icore.cpp
    iuicontroller.cpp
    iplugincontroller.cpp
    iprojectcontroller.cpp
    iproject.cpp
    ilanguagecontroller.cpp
    ilanguage.cpp
    idocumentcontroller.cpp
    istatus.cpp
    iruncontroller.cpp
    isession.cpp
    isourceformatter.cpp
    isourceformattercontroller.cpp
    contextmenuextension.cpp
    icompletionsettings.cpp
    iselectioncontroller.cpp
    idocumentationprovider.cpp
    idocumentation.cpp
    idocumentationcontroller.cpp
    idebugcontroller.cpp
    ipartcontroller.cpp
    launchconfigurationpage.cpp
    launchconfigurationtype.cpp
	ilauncher.cpp
	ilaunchconfiguration.cpp
	ilaunchmode.cpp
)


kde4_add_library(kdevplatforminterfaces SHARED ${kdevplatforminterfaces_LIB_SRCS})
target_link_libraries(kdevplatforminterfaces 
        ${KDE4_KIO_LIBS} 
        ${KDE4_KUTILS_LIBS} 
        ${KDE4_KPARTS_LIBS} 
        ${KDE4_KNOTIFYCONFIG_LIBS} 
        ${KDE4_KTEXTEDITOR_LIBS} 
        ${KDE4_THREADWEAVER_LIBRARIES} 
        ${QT_QTDESIGNER_LIBRARY} 
        ${KDE4_KROSSCORE_LIBS})
target_link_libraries(kdevplatforminterfaces LINK_INTERFACE_LIBRARIES 
        ${KDE4_KPARTS_LIBS} 
        ${KDE4_KTEXTEDITOR_LIBS} 
        ${QT_QTDESIGNER_LIBRARY})
set_target_properties(kdevplatforminterfaces PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatforminterfaces EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    iassistant.h
    context.h
    contextmenuextension.h
    iplugin.h
    icore.h
    iuicontroller.h
    iplugincontroller.h
    iprojectcontroller.h
    iproject.h
    ilanguagecontroller.h
    ilanguage.h
    idocument.h
    idocumentcontroller.h
    isourceformatter.h
    isourceformattercontroller.h
    interfacesexport.h
    istatus.h
    isession.h
    iruncontroller.h
    ilaunchconfiguration.h
    ilauncher.h
    launchconfigurationpage.h
    launchconfigurationtype.h
    icompletionsettings.h
    iselectioncontroller.h
    idocumentation.h
    idocumentationprovider.h
    idocumentationcontroller.h
    idebugcontroller.h
    ipartcontroller.h
    ilaunchmode.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/interfaces COMPONENT Devel
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
