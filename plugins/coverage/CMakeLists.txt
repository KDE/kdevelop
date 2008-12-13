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
    tests/viewstub.cpp
    tests/pluginstub.cpp)

kde4_add_library(kdevveritascoverage SHARED ${veritascoverage_SRCS})
target_link_libraries(kdevveritascoverage
    ${KDevPlatform_INTERFACES_LIBRARY}
    ${KDevPlatform_SHELL_LIBRARY}
    ${KDevPlatform_OUTPUTVIEW_LIBRARY}
    ${KDevPlatform_UTIL_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDevPlatform_INTERFACES_LIBRARY})
install(TARGETS kdevveritascoverage ${INSTALL_TARGETS_DEFAULT_ARGS})

qt4_automoc(coverageplugin.cpp)
kde4_add_plugin(kdevcoverage coverageplugin.cpp)
target_link_libraries(kdevcoverage
    kdevveritascoverage
    ${KDevPlatform_OUTPUTVIEW_LIBRARY}
    ${KDevPlatform_INTERFACES_LIBRARY}
    ${KDevPlatform_UTIL_LIBRARY}
    ${KDevPlatform_PROJECT_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KDEUI_LIBS})
install(TARGETS kdevcoverage DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevcoverage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcoverage.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcoverage )
install( PROGRAMS lcov_geninfo DESTINATION ${BIN_INSTALL_DIR} )
