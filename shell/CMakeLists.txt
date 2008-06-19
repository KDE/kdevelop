
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9501 )
add_subdirectory(settings)
add_subdirectory(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sublime
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/language/editor
    ${CMAKE_SOURCE_DIR}/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/outputview
    ${CMAKE_SOURCE_DIR}/util
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
    statusbar.cpp
    runcontroller.cpp
    savedialog.cpp
)


kde4_add_library(kdevplatformshell SHARED ${kdevplatformshell_LIB_SRCS})
target_link_libraries(kdevplatformshell ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformproject kdevplatformlanguage sublime kdevplatformutil )
set_target_properties(kdevplatformshell PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformshell ${INSTALL_TARGETS_DEFAULT_ARGS} )

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
    shellexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/shell
)
