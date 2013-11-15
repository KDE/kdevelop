project(qmakebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9039 )

include_directories( ${qmakebuilder_BINARY_DIR} )

########### next target ###############

set(qmakebuilder_LIB_SRCS
    qmakebuilder.cpp
    qmakejob.cpp
    ../qmakeconfig.cpp
)


kde4_add_plugin(kdevqmakebuilder ${qmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevqmakebuilder
        ${KDE4_KDEUI_LIBS}
        ${QT_QTDESIGNER_LIBRARY}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevqmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############
set( qmakebuilder_cfg_SRCS
    qmakebuilderpreferences.cpp
    ../qmakebuilddirchooser.cpp
    ../qmakebuilddirchooserdialog.cpp
    ../qmakeconfig.cpp
)

set( qmakebuilder_cfg_UI
    qmakeconfig.ui
    ../qmakebuilddirchooser.ui
)


kde4_add_ui_files( qmakebuilder_cfg_SRCS ${qmakebuilder_cfg_UI} )
kde4_add_kcfg_files( qmakebuilder_cfg_SRCS qmakebuilderconfig.kcfgc )
kde4_add_plugin( kcm_kdev_qmakebuilder ${qmakebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_qmakebuilder ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} )

########### install files ###############
configure_file(kdevqmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES iqmakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/qmake COMPONENT Devel)
install( TARGETS kcm_kdev_qmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_qmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevqmakebuilder )


