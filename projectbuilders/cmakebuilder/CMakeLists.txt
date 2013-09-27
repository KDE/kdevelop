project(cmakebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9032 )
include_directories(
    ${cmakebuilder_SOURCE_DIR}
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    )


########### next target ###############

set(cmakebuilder_LIB_SRCS
    cmakebuilder.cpp
    cmakejob.cpp
    prunejob.cpp
)

kde4_add_kcfg_files( cmakebuilder_LIB_SRCS cmakebuilderconfig.kcfgc )
kde4_add_plugin(kdevcmakebuilder ${cmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevcmakebuilder
    kdev4cmakecommon
        ${KDE4_KIO_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevcmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )
configure_file(kdevcmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
########### next target ###############

set( cmakebuilder_cfg_SRCS
    cmakebuilderpreferences.cpp
)

set( cmakebuilder_cfg_UI
    cmakebuildersettings.ui
)


kde4_add_ui_files( cmakebuilder_cfg_SRCS ${cmakebuilder_cfg_UI} )
kde4_add_kcfg_files( cmakebuilder_cfg_SRCS cmakebuilderconfig.kcfgc )

kde4_add_plugin( kcm_kdev_cmakebuilder ${cmakebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_cmakebuilder ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )
install( TARGETS kcm_kdev_cmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_cmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


