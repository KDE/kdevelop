
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/plugins/outputviews/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    makebuilder.cpp
    makeitem.cpp
    makeitemfactory.cpp
)

kde4_automoc(${kdevmakebuilder_LIB_SRCS})
kde4_add_plugin(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder
        ${KDE4_KDECORE_LIBS}
        ${QT_QTDESIGNER_LIBRARY}
        kdevplatforminterfaces
        kdevplatformproject
        kdevoutputviewinterfaces
)

install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )


set( makebuilder_cfg_SRCS
    makebuilderpreferences.cpp
)

set( makebuilder_cfg_UI
    makeconfig.ui
)

kde4_automoc(${makebuilder_cfg_SRCS})
kde4_add_ui_files( makebuilder_cfg_SRCS ${makebuilder_cfg_UI} )
kde4_add_kcfg_files( makebuilder_cfg_SRCS makebuilderconfig.kcfgc )
kde4_add_plugin( kcm_kdev_makebuilder ${makebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_makebuilder ${KDE4_KUTILS_LIBS} kdevplatforminterfaces kdevplatformproject )

install( TARGETS kcm_kdev_makebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES imakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/make)
install( FILES kcm_kdev_makebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


