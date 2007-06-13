include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
)

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
)

kde4_automoc(${kdevplatforminterfaces_LIB_SRCS})

kde4_add_library(kdevplatforminterfaces SHARED ${kdevplatforminterfaces_LIB_SRCS})
target_link_libraries(kdevplatforminterfaces ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} )
set_target_properties(kdevplatforminterfaces PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatforminterfaces DESTINATION ${LIB_INSTALL_DIR} )

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
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/interfaces
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
