
include_directories(
    ${CMAKE_SOURCE_DIR}
)

########### Plugin Selector
set( pluginsettings_cfg_SRCS
     pluginpreferences.cpp
)

kde4_add_plugin( kcm_kdev_pluginsettings ${pluginsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_pluginsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell )

########### UI settings
set( uisettings_cfg_SRCS
    uipreferences.cpp
)

set( uisettings_cfg_UI
    uiconfig.ui
)

kde4_add_ui_files( uisettings_cfg_SRCS ${uisettings_cfg_UI} )
kde4_add_kcfg_files( uisettings_cfg_SRCS uiconfig.kcfgc )
kde4_add_plugin( kcm_kdev_uisettings ${uisettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_uisettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

########### Background Parser

set( bgsettings_cfg_SRCS
    bgpreferences.cpp
)

set( bgsettings_cfg_UI
    bgsettings.ui
)

kde4_add_ui_files( bgsettings_cfg_SRCS ${bgsettings_cfg_UI} )
kde4_add_kcfg_files( bgsettings_cfg_SRCS bgconfig.kcfgc )
kde4_add_plugin( kcm_kdev_bgsettings ${bgsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_bgsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformlanguage)

########### Background Parser

set( ccsettings_cfg_SRCS
    ccpreferences.cpp
)

set( ccsettings_cfg_UI
    ccsettings.ui
)

kde4_add_ui_files( ccsettings_cfg_SRCS ${ccsettings_cfg_UI} )
kde4_add_kcfg_files( ccsettings_cfg_SRCS ccconfig.kcfgc )
kde4_add_plugin( kcm_kdev_ccsettings ${ccsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_ccsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatforminterfaces kdevplatformshell ${KDE4_KTEXTEDITOR_LIBS})

########### Project Configuration

set( projectsettings_cfg_SRCS
    projectpreferences.cpp
)

set( projectsettings_cfg_UI
    projectsettings.ui
)

kde4_add_ui_files( projectsettings_cfg_SRCS ${projectsettings_cfg_UI} )
kde4_add_kcfg_files( projectsettings_cfg_SRCS projectconfig.kcfgc )
kde4_add_plugin( kcm_kdev_projectsettings ${projectsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_projectsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell)

########### Run Options

set( runsettings_cfg_SRCS
    runpreferences/projectitemlineedit.cpp
    runpreferences/runpreferences.cpp
    runpreferences/runconfig.cpp
)

set( runsettings_cfg_UI
    runpreferences/runsettings.ui
    runpreferences/runconfig.ui
)

kde4_add_ui_files( runsettings_cfg_SRCS ${runsettings_cfg_UI} )
kde4_add_plugin( kcm_kdev_runsettings ${runsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_runsettings ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell kdevplatformutil kdevplatformproject kdevplatforminterfaces)

########### Environment widget

set( envsettings_cfg_SRCS
  environmentwidget.cpp
  environmentgroupmodel.cpp
  environmentpreferences.cpp
)

set( envsettings_cfg_UI
  environmentwidget.ui
)

kde4_add_ui_files( envsettings_cfg_SRCS ${envsettings_cfg_UI} )
kde4_add_plugin( kcm_kdev_envsettings ${envsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformutil)

install( TARGETS 
        kcm_kdev_projectsettings 
        kcm_kdev_runsettings 
        kcm_kdev_ccsettings
        kcm_kdev_bgsettings
        kcm_kdev_pluginsettings
        kcm_kdev_envsettings 
        kcm_kdev_uisettings 
        DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES 
        kcm_kdev_uisettings.desktop 
        kcm_kdev_runsettings.desktop
        kcm_kdev_envsettings.desktop
        kcm_kdev_pluginsettings.desktop
        kcm_kdev_projectsettings.desktop
        kcm_kdev_bgsettings.desktop
        kcm_kdev_ccsettings.desktop 
        DESTINATION ${SERVICES_INSTALL_DIR} )
