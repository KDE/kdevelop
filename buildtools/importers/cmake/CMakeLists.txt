add_subdirectory(tests)

include_directories(
 ${CMAKE_SOURCE_DIR}/lib 
 ${CMAKE_SOURCE_DIR}/lib/projectmanager/interfaces
 ${CMAKE_SOURCE_DIR}/lib/project
 ${CMAKE_SOURCE_DIR}/lib/util
)

set( cmakecommon_SRCS 
  cmakemodelitems.cpp
  cmakexmlparser.cpp
 )

set( cmakeimporter_SRCS
  cmakeimporter.cpp
)

set( cmakesettings_SRCS
  cmakepreferences.cpp
)

set( cmakesettings_UI
  cmakebuildsettings.ui
)

remove_definitions( -DQT_NO_STL )

kde4_automoc( ${cmakecommon_SRCS} )
kde4_add_kcfg_files( cmakecommon_SRCS cmakeconfig.kcfgc )
kde4_add_library( kdevcmakecommon SHARED ${cmakecommon_SRCS} )
target_link_libraries( kdevcmakecommon ${KDE4_KDECORE_LIBS}
                       kdevplatform )

kde4_automoc( ${cmakeimporter_SRCS} )
kde4_add_plugin( kdevcmakeimporter ${cmakeimporter_SRCS} )
target_link_libraries( kdevcmakeimporter ${KDE4_KDECORE_LIBS} 
                       kdevplatform kdevcmakecommon)

kde4_automoc( ${cmakesettings_SRCS} )
kde4_add_ui_files( cmakesettings_SRCS ${cmakesettings_UI} )
kde4_add_plugin( kcm_kdevcmake_settings ${cmakesettings_SRCS} )
target_link_libraries( kcm_kdevcmake_settings ${KDE4_KUTILS_LIBS} kdevplatform kdevcmakecommon )



kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kdevcmakeimporter )
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdevcmake_settings )
install(TARGETS kdevcmakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kcm_kdevcmake_settings DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kdevcmakecommon DESTINATION ${LIB_INSTALL_DIR} )
install(FILES kdevcmakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kcm_kdevcmake_settings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevcmakeimporter )

