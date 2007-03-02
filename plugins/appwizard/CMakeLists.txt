include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )


########### next target ###############

set(kdevappwizard_PART_SRCS
    appwizardpart.cpp
    appwizardfactory.cpp
    appwizarddialog.cpp
    projectselectionpage.cpp
    projecttemplatesmodel.cpp
)

set(kdevappwizard_PART_UI
    projectselectionpage.ui
)

kde4_add_ui_files(kdevappwizard_PART_SRCS ${kdevappwizard_PART_UI})
kde4_automoc(${kdevappwizard_PART_SRCS})
kde4_add_plugin(kdevappwizard ${kdevappwizard_PART_SRCS})
target_link_libraries(kdevappwizard ${KDE4_KDECORE_LIBS} kdevplatforminterfaces)

install(TARGETS kdevappwizard DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevappwizard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevappwizard.rc DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
