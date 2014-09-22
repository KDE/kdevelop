set(KDevPlatformInterfaces_LIB_SRCS
    iassistant.cpp
    context.cpp
    configpage.cpp
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

add_library(KDevPlatformInterfaces ${KDevPlatformInterfaces_LIB_SRCS})
add_library(KDev::Interfaces ALIAS KDevPlatformInterfaces)
generate_export_header(KDevPlatformInterfaces EXPORT_FILE_NAME interfacesexport.h)
target_link_libraries(KDevPlatformInterfaces
LINK_PUBLIC
    KF5::ConfigCore
    KF5::Parts
    KF5::TextEditor
    KF5::I18n
    KF5::Service
LINK_PRIVATE
    KF5::IconThemes
    KF5::NotifyConfig
    KF5::KIOWidgets
    KF5::KCMUtils
    KF5::ThreadWeaver
)
target_include_directories(KDevPlatformInterfaces INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kdevplatform>")
# TODO: this is ugly (sort-of cyclic dependecy):
target_include_directories(KDevPlatformInterfaces PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../util") # for utilexport.h

set_target_properties(KDevPlatformInterfaces PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION} EXPORT_NAME Interfaces)
install(TARGETS KDevPlatformInterfaces EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    iassistant.h
    context.h
    configpage.h
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
    ${CMAKE_CURRENT_BINARY_DIR}/interfacesexport.h
    ${CMAKE_CURRENT_BINARY_DIR}/ipluginversion.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/interfaces COMPONENT Devel
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
