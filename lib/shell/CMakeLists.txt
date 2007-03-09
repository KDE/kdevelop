include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language
    ${CMAKE_SOURCE_DIR}/lib/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
)

set(kdevplatformshell_LIB_SRCS
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
    partdocument.cpp
    languagecontroller.cpp
    language.cpp
    # This will be changed once mattr has thought about configuration.
     ../kdevconfig.cpp
     ../kdevconfigskeleton.cpp
     ../domutil.cpp
     ../kdevcmodule.cpp
)

kde4_automoc(${kdevplatformshell_LIB_SRCS})

kde4_add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES} ${BERKELEY_DB_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformproject kdevplatformeditor kdevplatformlanguage sublime )
set_target_properties(kdevplatformshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformshell DESTINATION ${LIB_INSTALL_DIR} )
