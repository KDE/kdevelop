
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util 
    ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    kdevmakebuilder.cpp
)

kde4_automoc(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
kde4_add_plugin(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder ${KDE4_KDECORE_LIBS} kdevplatform)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevmakebuilder)
install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )


set( makebuilder_cfg_SRCS
    makebuilderpreferences.cpp
)

set( makebuilder_cfg_UI
    makeconfig.ui  
)

kde4_automoc( makebuilder_cfg ${makebuilder_cfg_SRCS} )
kde4_add_ui_files( makebuilder_cfg_SRCS ${makebuilder_cfg_UI} )
kde4_add_kcfg_files( makebuilder_cfg_SRCS makebuilderconfig.kcfgc )
kde4_add_plugin( kcm_kdev_makebuilder ${makebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_makebuilder ${KDE4_KUTILS_LIBS} kdevplatform )
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdev_makebuilder )
install( TARGETS kcm_kdev_makebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_kdev_makebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


