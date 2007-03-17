include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(kdevplatforminterfaces_LIB_SRCS
    iplugin.cpp
    icore.cpp
    iuicontroller.cpp
    iplugincontroller.cpp
    iprojectcontroller.cpp
    iproject.cpp
    ilanguagecontroller.cpp
    ilanguage.cpp
)

kde4_automoc(${kdevplatforminterfaces_LIB_SRCS})

kde4_add_library(kdevplatforminterfaces SHARED ${kdevplatforminterfaces_LIB_SRCS})
target_link_libraries(kdevplatforminterfaces ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES} ${BERKELEY_DB_LIBRARIES} )
set_target_properties(kdevplatforminterfaces PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatforminterfaces DESTINATION ${LIB_INSTALL_DIR} )

install(FILES
    iplugin.h
    icore.h
    iuicontroller.h
    iplugincontroller.h
    iprojectcontroller.h
    iproject.h
    ilanguagecontroller.h
    ilanguage.h
    iextension.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/interfaces
)

install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
