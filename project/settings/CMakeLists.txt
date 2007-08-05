include_directories(
    ${CMAKE_CURRENT_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
)

set( environment_cfg_SRCS
    environmentpreferences.cpp
    environmentwidget.cpp
)

set( environment_cfg_UI
    environmentwidget.ui
)

kde4_add_ui_files( environment_cfg_SRCS ${environment_cfg_UI} )
kde4_add_plugin( kcm_kdev_envsettings ${environment_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings kdevplatformproject kdevplatformutil ${KDE4_KUTILS_LIBS} )

install( TARGETS kcm_kdev_envsettings DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kcm_kdev_envsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
