project(coverage)
add_subdirectory(tests)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

########### next target ###############

set(veritascoverage_SRCS
    coveredfile.cpp
    annotationmodel.cpp
    lcovinfoparser.cpp
    annotationmanager.cpp
    lcovjob.cpp
    covoutputmodel.cpp
    covoutputdelegate.cpp
    reportwidget.cpp
    reportmodel.cpp
    reportproxymodel.cpp
    drilldownview.cpp
    colorrange.cpp
    discretecolorrange.cpp
    gradientcolorrange.cpp
    tests/viewstub.cpp
    tests/pluginstub.cpp)

kde4_add_ui_files(veritascoverage_SRCS reportwidget.ui)

kde4_add_library(kdevveritascoverage SHARED ${veritascoverage_SRCS})
target_link_libraries(kdevveritascoverage
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_SHELL_LIBRARY}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY})
install(TARGETS kdevveritascoverage ${INSTALL_TARGETS_DEFAULT_ARGS})

qt4_automoc(coverageplugin.cpp)
kde4_add_plugin(kdevcoverage coverageplugin.cpp)
target_link_libraries(kdevcoverage
    kdevveritascoverage
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KDEUI_LIBS})
install(TARGETS kdevcoverage DESTINATION ${PLUGIN_INSTALL_DIR})

if( NOT KDE_VERSION VERSION_LESS "4.2.2" )

  set(lcovsettings_cfg_SRCS
      lcovprefs.cpp
      lcovprefswidget.cpp
      lcovsettingsbase.cpp
      stoppointwidget.cpp
      colorrangebar.cpp
      colorrange.cpp
      discretecolorrange.cpp
      gradientcolorrange.cpp)
  
  kde4_add_ui_files( lcovsettings_cfg_SRCS lcovprefswidget.ui stoppointwidget.ui)
  kde4_add_kcfg_files( lcovsettings_cfg_SRCS lcovconfig.kcfgc )
  kde4_add_plugin( kcm_kdev_lcovsettings ${lcovsettings_cfg_SRCS} )
  target_link_libraries( kcm_kdev_lcovsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )
  install( TARGETS kcm_kdev_lcovsettings DESTINATION ${PLUGIN_INSTALL_DIR})
  install( FILES   kcm_kdev_lcovsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

endif( NOT KDE_VERSION VERSION_LESS "4.2.2" )

########### install files ###############

install( FILES kdevcoverage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcoverage.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcoverage )
