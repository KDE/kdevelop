find_package(Threads)
find_library(RT_LIBRARY rt)

add_subdirectory(tests)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9502 )
set(kdevplatforminterfaces_LIB_SRCS
    foregroundlock.cpp
    ipatchsource.cpp
    ipatchexporter.cpp
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
    isessionlock.cpp
    isourceformatter.cpp
    isourceformattercontroller.cpp
    contextmenuextension.cpp
    icompletionsettings.cpp
    iselectioncontroller.cpp
    idocumentationprovider.cpp
    idocumentationproviderprovider.cpp
    idocumentation.cpp
    idocumentationcontroller.cpp
    idebugcontroller.cpp
    ipartcontroller.cpp
    launchconfigurationpage.cpp
    launchconfigurationtype.cpp
    ilauncher.cpp
    ilaunchconfiguration.cpp
    ilaunchmode.cpp
    iprojectprovider.cpp
    ibuddydocumentfinder.cpp
    itemplateprovider.cpp
    itestsuite.cpp
    itestcontroller.cpp
    ilanguagecheck.cpp
    ilanguagecheckprovider.cpp
)
configure_file(ipluginversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/ipluginversion.h)

add_library(kdevplatforminterfaces SHARED ${kdevplatforminterfaces_LIB_SRCS})
target_link_libraries(kdevplatforminterfaces LINK_PUBLIC
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)
target_link_libraries(kdevplatforminterfaces LINK_PRIVATE
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDE4_KROSSCORE_LIBS}
)
if( RT_LIBRARY )
    target_link_libraries( kdevplatforminterfaces LINK_PRIVATE ${RT_LIBRARY} )
endif( RT_LIBRARY )

if( Threads_FOUND )
    target_link_libraries( kdevplatforminterfaces LINK_PRIVATE ${CMAKE_THREAD_LIBS_INIT} )
endif( Threads_FOUND )

set_target_properties(kdevplatforminterfaces PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatforminterfaces EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    foregroundlock.h
    iassistant.h
    context.h
    contextmenuextension.h
    iplugin.h
    icore.h
    iuicontroller.h
    ipatchdocument.h
    ipatchexporter.h
    ipatchsource.h
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
    isessionlock.h
    iruncontroller.h
    ilaunchconfiguration.h
    ilauncher.h
    launchconfigurationpage.h
    launchconfigurationtype.h
    icompletionsettings.h
    iselectioncontroller.h
    idocumentation.h
    idocumentationprovider.h
    idocumentationproviderprovider.h
    idocumentationcontroller.h
    idebugcontroller.h
    ipartcontroller.h
    ilaunchmode.h
    iprojectprovider.h
    ilanguagecheck.h
    ilanguagecheckprovider.h
    ibuddydocumentfinder.h
    itemplateprovider.h
    itestsuite.h
    itestcontroller.h
    ${CMAKE_CURRENT_BINARY_DIR}/ipluginversion.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/interfaces COMPONENT Devel
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
