add_subdirectory(settings)

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
    mainwindow_p.cpp
    plugincontroller.cpp
    profileengine.cpp
    profile.cpp
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
)

kde4_automoc(${kdevplatformshell_LIB_SRCS})

kde4_add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformproject kdevplatformeditor kdevplatformlanguage sublime )
set_target_properties(kdevplatformshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformshell DESTINATION ${LIB_INSTALL_DIR} )

install(FILES
    mainwindow.h
    plugincontroller.h
    profileengine.h
    profile.h
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
    language.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/shell
)
