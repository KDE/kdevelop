include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(koncreteinterfaces_LIB_SRCS
    iplugin.cpp
    icore.cpp
    iuicontroller.cpp
    iplugincontroller.cpp
    iprojectcontroller.cpp
    iproject.cpp
)

kde4_automoc(${koncreteinterfaces_LIB_SRCS})

kde4_add_library(koncreteinterfaces SHARED ${koncreteinterfaces_LIB_SRCS})
target_link_libraries(koncreteinterfaces ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES} ${BERKELEY_DB_LIBRARIES} )
set_target_properties(koncreteinterfaces PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS koncreteinterfaces DESTINATION ${LIB_INSTALL_DIR} )


install(FILES
    kdevelopplugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
