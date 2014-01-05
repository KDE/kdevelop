
include_directories(
    ${KDevPlatform_SOURCE_DIR}
)

########### Plugin Selector
set( pluginsettings_cfg_SRCS
     pluginpreferences.cpp
)

add_library( kcm_kdev_pluginsettings MODULE ${pluginsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_pluginsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell )

########### UI settings
set( uisettings_cfg_SRCS
    uipreferences.cpp
)

set( uisettings_cfg_UI
    uiconfig.ui
)

qt5_wrap_ui( uisettings_cfg_SRCS ${uisettings_cfg_UI} )
kconfig_add_kcfg_files( uisettings_cfg_SRCS uiconfig.kcfgc )
add_library( kcm_kdev_uisettings MODULE ${uisettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_uisettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

########### Background Parser

set( bgsettings_cfg_SRCS
    bgpreferences.cpp
)

set( bgsettings_cfg_UI
    bgsettings.ui
)

qt5_wrap_ui( bgsettings_cfg_SRCS ${bgsettings_cfg_UI} )
kconfig_add_kcfg_files( bgsettings_cfg_SRCS bgconfig.kcfgc )
add_library( kcm_kdev_bgsettings MODULE ${bgsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_bgsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformlanguage)

########### Background Parser

set( ccsettings_cfg_SRCS
    ccpreferences.cpp
)

set( ccsettings_cfg_UI
    ccsettings.ui
)

qt5_wrap_ui( ccsettings_cfg_SRCS ${ccsettings_cfg_UI} )
kconfig_add_kcfg_files( ccsettings_cfg_SRCS ccconfig.kcfgc )
add_library( kcm_kdev_ccsettings MODULE ${ccsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_ccsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatforminterfaces kdevplatformshell ${KDE4_KTEXTEDITOR_LIBS})

########### Project Configuration

set( projectsettings_cfg_SRCS
    projectpreferences.cpp
)

set( projectsettings_cfg_UI
    projectsettings.ui
)

qt5_wrap_ui( projectsettings_cfg_SRCS ${projectsettings_cfg_UI} )
kconfig_add_kcfg_files( projectsettings_cfg_SRCS projectconfig.kcfgc )
add_library( kcm_kdev_projectsettings MODULE ${projectsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_projectsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell)

########### Environment widget

set( envsettings_cfg_SRCS
  environmentwidget.cpp
  environmentgroupmodel.cpp
  environmentpreferences.cpp
)

set( envsettings_cfg_UI
  environmentwidget.ui
)

qt5_wrap_ui( envsettings_cfg_SRCS ${envsettings_cfg_UI} )
add_library( kcm_kdev_envsettings MODULE ${envsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformutil)

########## Formatter Settings
set(kcm_kdevsourceformattersettings_SRCS
    editstyledialog.cpp
    sourceformattersettings.cpp
)

set(kcm_kdevsourceformattersettings_UI
    editstyledialog.ui
    sourceformattersettings.ui
)

qt5_wrap_ui(kcm_kdevsourceformattersettings_SRCS ${kcm_kdevsourceformattersettings_UI} )
add_library(kcm_kdevsourceformattersettings MODULE ${kcm_kdevsourceformattersettings_SRCS})
target_link_libraries(kcm_kdevsourceformattersettings
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS} kdevplatformshell kdevplatformutil kdevplatformlanguage)

install(TARGETS  DESTINATION ${PLUGIN_INSTALL_DIR})


########## Install TARGETS/Files

install( TARGETS 
        kcm_kdev_projectsettings 
        kcm_kdev_ccsettings
        kcm_kdev_bgsettings
        kcm_kdev_pluginsettings
        kcm_kdev_envsettings 
        kcm_kdev_uisettings 
        kcm_kdevsourceformattersettings
        DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES 
        kcm_kdev_uisettings.desktop 
        kcm_kdev_envsettings.desktop
        kcm_kdev_pluginsettings.desktop
        kcm_kdev_projectsettings.desktop
        kcm_kdev_bgsettings.desktop
        kcm_kdev_ccsettings.desktop 
        kcm_kdevsourceformattersettings.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} )
