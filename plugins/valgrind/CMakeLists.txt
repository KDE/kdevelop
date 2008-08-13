
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9002 )

########### next target ###############

set( valgrindsettings_SRCS
    valgrindpreferences.cpp
)

set( valgrindsettings_UI
    valgrindsettings.ui
)

kde4_add_ui_files( valgrindsettings_SRCS ${valgrindsettings_UI} )
kde4_add_kcfg_files( valgrindsettings_SRCS valgrindconfig.kcfgc )
kde4_add_plugin( kcm_kdev_valgrindsettings ${valgrindsettings_SRCS} )
target_link_libraries( kcm_kdev_valgrindsettings ${KDE4_KUTILS_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDE4_KIO_LIBS} )

install( TARGETS kcm_kdev_valgrindsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_valgrindsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### next target ###############

set(kdevvalgrind_PART_SRCS
    valgrindplugin.cpp
    valgrindmodel.cpp
    valgrindcontrol.cpp
    valgrindwidget.cpp
    valgrindtree.cpp
    #massifmodel.cpp
    #massifgraph.cpp
#modeltest.cpp
)

kde4_add_ui_files(kdevvalgrind_PART_SRCS ${kdevvalgrind_PART_UI})

kde4_add_plugin(kdevvalgrind ${kdevvalgrind_PART_SRCS})
target_link_libraries(kdevvalgrind
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${QT_QTNETWORK_LIBRARY})

install(TARGETS kdevvalgrind DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevvalgrind.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevvalgrind.rc DESTINATION ${DATA_INSTALL_DIR}/kdevvalgrind )
