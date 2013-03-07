
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9519 )


########### next target ###############

set(kdevproblemreporter_PART_SRCS
    problemreporterplugin.cpp
    problemwidget.cpp
    problemmodel.cpp
    problemhighlighter.cpp
    watcheddocumentset.cpp
#modeltest.cpp
)

kde4_add_plugin(kdevproblemreporter ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KPARTS_LIBS} kdevplatformlanguage kdevplatforminterfaces kdevplatformutil kdevplatformproject)

install(TARGETS kdevproblemreporter DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevproblemreporter.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevproblemreporter.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevproblemreporter.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevproblemreporter.rc DESTINATION ${DATA_INSTALL_DIR}/kdevproblemreporter)
