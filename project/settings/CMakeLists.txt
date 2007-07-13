include_directories(
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
)

set( environment_cfg_SRCS
    envpreferences.cpp
)

kde4_add_plugin( kcm_kdev_envsettings ${environment_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings kdevplatformproject kdevplatformutil ${KDE4_KUTILS_LIBS} )

install( TARGETS kcm_kdev_envsettings DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kcm_kdev_envsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
