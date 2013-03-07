project(makebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9037 )



########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    makebuilder.cpp
    makejob.cpp
)


kde4_add_plugin(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KTEXTEDITOR_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
)

install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )


set( makebuilder_cfg_SRCS
    makebuilderpreferences.cpp
)

set( makebuilder_cfg_UI
    makeconfig.ui
)


kde4_add_ui_files( makebuilder_cfg_SRCS ${makebuilder_cfg_UI} )
kde4_add_kcfg_files( makebuilder_cfg_SRCS makebuilderconfig.kcfgc )
kde4_add_plugin( kcm_kdev_makebuilder ${makebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_makebuilder ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )

install( TARGETS kcm_kdev_makebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES imakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/make COMPONENT Devel)
install( FILES kcm_kdev_makebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
configure_file(kdevmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
# install( FILES kdevmakebuilder.rc DESTINATION ${DATA_INSTALL_DIR}/kdevmakebuilder )
