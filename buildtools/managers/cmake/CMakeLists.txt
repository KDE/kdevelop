
include_directories(
 ${KDEVPLATFORM_INCLUDE_DIR}
 ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
 ${KDEVPLATFORM_INCLUDE_DIR}/projectmanager/interfaces
 ${KDEVPLATFORM_INCLUDE_DIR}/project
 ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
 ${KDEVPLATFORM_INCLUDE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/parser
)
add_subdirectory(tests)

set( cmakecommon_SRCS
  cmakemodelitems.cpp
  #cmakexmlparser.cpp
  parser/cmListFileLexer.c
  parser/astfactory.cpp
  parser/cmakelistsparser.cpp
  parser/cmakeast.cpp
 )

set( cmakemanager_SRCS
  parser/cmakeprojectvisitor.cpp
  cmakemanager.cpp
)

set( cmakesettings_SRCS
  cmakepreferences.cpp
)

set( cmakesettings_UI
  cmakebuildsettings.ui
)

remove_definitions( -DQT_NO_STL )


kde4_add_kcfg_files( cmakecommon_SRCS cmakeconfig.kcfgc )
kde4_add_library( kdevcmakecommon SHARED ${cmakecommon_SRCS} )
target_link_libraries( kdevcmakecommon ${KDE4_KDEUI_LIBS}
                       ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} )
set_target_properties(kdevcmakecommon PROPERTIES VERSION 1.0.0 SOVERSION 1 )


kde4_add_plugin( kdevcmakemanager ${cmakemanager_SRCS} )
target_link_libraries( kdevcmakemanager ${KDE4_KDEUI_LIBS}
                       ${KDEVPLATFORM_INTERFACES_LIBRARY} kdevcmakecommon ${KDEVPLATFORM_PROJECT_LIBRARY})


kde4_add_ui_files( cmakesettings_SRCS ${cmakesettings_UI} )
kde4_add_plugin( kcm_kdevcmake_settings ${cmakesettings_SRCS} )
target_link_libraries( kcm_kdevcmake_settings ${KDE4_KUTILS_LIBS}
                       ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} kdevcmakecommon )





install(TARGETS kdevcmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kcm_kdevcmake_settings DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kdevcmakecommon DESTINATION ${LIB_INSTALL_DIR} )
install(FILES kdevcmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kcm_kdevcmake_settings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevcmakemanager )

