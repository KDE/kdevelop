add_subdirectory(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9502 )
set(kdevplatforminterfaces_LIB_SRCS
    context.cpp
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
    irun.cpp
    iruncontroller.cpp
    irunprovider.cpp
    contextmenuextension.cpp
    scripting/scriptdocument.cpp
    scripting/scripttools.cpp
    scripting/scriptingmanager.cpp
)


kde4_add_library(kdevplatforminterfaces SHARED ${kdevplatforminterfaces_LIB_SRCS})
target_link_libraries(kdevplatforminterfaces ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} ${KDE4_KROSSCORE_LIBS})
set_target_properties(kdevplatforminterfaces PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatforminterfaces ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    context.h
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
    iextension.h
    interfacesexport.h
    istatus.h
    irun.h
    iruncontroller.h
    irunprovider.h
    contextmenuextension.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/interfaces
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
