include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
)

set(koncreteshell_LIB_SRCS
    mainwindow.cpp
    plugincontroller.cpp
    profileengine.cpp
    profile.cpp
    shellextension.cpp
    core.cpp
    uicontroller.cpp
    projectcontroller.cpp
    project.cpp
    partcontroller.cpp
    partcontroller.h
    # This will be changed once mattr has thought about configuration.
     ../kdevconfig.cpp
     ../kdevconfigskeleton.cpp
)

kde4_automoc(${koncreteshell_LIB_SRCS})

kde4_add_library(koncreteshell SHARED ${koncreteshell_LIB_SRCS})
target_link_libraries(koncreteshell ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES} ${BERKELEY_DB_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} koncreteinterfaces koncreteproject sublime )
set_target_properties(koncreteshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS koncreteshell DESTINATION ${LIB_INSTALL_DIR} )
