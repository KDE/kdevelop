add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9000 )

include_directories(lib)
add_subdirectory(lib)

########### next target ###############
set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
)

kde4_add_plugin(kdevastyle ${kdevastyle_PART_SRCS})
target_link_libraries(kdevastyle
    astylewrapper
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})

install(TARGETS kdevastyle DESTINATION ${PLUGIN_INSTALL_DIR} )

######### Config KCModule ############

set( astyle_preferences_SRCS
    astyle_preferences.cpp
 )

set( astyle_preferences_UI
    astyle_preferences.ui
)

kde4_add_ui_files( astyle_preferences_SRCS ${astyle_preferences_UI} )
kde4_add_plugin( kcm_kdev_astyle_preferences ${astyle_preferences_SRCS} )
target_link_libraries( kcm_kdev_astyle_preferences
    astylewrapper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS})

install( TARGETS kcm_kdev_astyle_preferences DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_astyle_preferences.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### install files ###############

install( FILES kdevastyle.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevastyle.rc DESTINATION ${DATA_INSTALL_DIR}/kdevastyle )