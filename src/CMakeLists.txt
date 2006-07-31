
add_subdirectory(profiles)
add_subdirectory(profileengine)
# add_subdirectory(newui)

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
    )


########### next target ###############

set(kdevshell_LIB_SRCS
    core.cpp
    documentationpart.cpp
    editorproxy.cpp
    generalinfowidget.cpp
    languageselectwidget.cpp
    mainwindowshare.cpp
    partselectwidget.cpp
    plugincontroller.cpp
    projectsession.cpp
    splashscreen.cpp
    statusbar.cpp
    toplevel.cpp
    shellextension.cpp
    simplemainwindow.cpp
    documentcontroller.cpp
    partcontroller.cpp
    projectcontroller.cpp
    languagecontroller.cpp
)

set(kdevshell_UI
    generalinfowidgetbase.ui
)

set(kdevshell_UI3
    mimewarningdialog.ui
)

kde4_automoc(${kdevshell_LIB_SRCS})
kde4_add_ui_files(kdevshell_LIB_SRCS ${kdevshell_UI})
kde4_add_ui3_files(kdevshell_LIB_SRCS ${kdevshell_UI3})
kde4_add_library(kdevshell SHARED ${kdevshell_LIB_SRCS})
kde4_add_kcfg_files(kdevshell_LIB_SRCS kdevelopconfig.kcfgc )
target_link_libraries(kdevshell ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdevinterfaces kdevutil
    kdevwidgets ${KDE4_KHTML_LIBS} ktexteditor knotifyconfig)
set_target_properties(kdevshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevshell DESTINATION ${LIB_INSTALL_DIR} )

### Configuration module for general kdevelop settings

set( kdev_config_SRC
    kdeveloppreferences.cpp
)
set( kdevelop_UI
    settingswidget.ui
)
kde4_automoc( ${kdev_config_SRC} )
kde4_add_ui_files( kdev_config_SRC ${kdevelop_UI} )
kde4_add_kcfg_files(kdev_config_SRC kdevelopconfig.kcfgc )
kde4_add_plugin(kcm_kdevelop_settings ${kdev_config_SRC})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdevelop_settings )
target_link_libraries(kcm_kdevelop_settings ${KDE4_KUTILS_LIBS} kdevinterfaces )
install(TARGETS kcm_kdevelop_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcm_kdevelop_settings.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
)



kde4_automoc(${kdevelop_SRCS})
kde4_add_executable(kdevelop ${kdevelop_SRCS})
target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} kdevshell)
install(TARGETS kdevelop DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kdevassistant_SRCS
    main_assistant.cpp
    kdevassistantextension.cpp
)

kde4_automoc(${kdevassistant_SRCS})
kde4_add_executable(kdevassistant ${kdevassistant_SRCS})
target_link_libraries(kdevassistant ${KDE4_KDECORE_LIBS} kdevshell)
install(TARGETS kdevassistant DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install(FILES
    core.h documentationpart.h editorproxy.h
    generalinfowidget.h languageselectwidget.h mainwindowshare.h
    partselectwidget.h plugincontroller.h
    projectsession.h shellextension.h splashscreen.h statusbar.h
    toplevel.h  simplemainwindow.h documentcontroller.h shellexport.h
    partcontroller.h projectcontroller.h languagecontroller.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/shell )

install( FILES kdeveloprc kdevassistantrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc kdevelop.notifyrc data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
install( FILES projectprofiles DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles )
install( FILES kdevassistantui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevassistant )


