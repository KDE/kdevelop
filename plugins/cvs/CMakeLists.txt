
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9513 )

add_subdirectory(tests)


########### next target ###############

set(kdevcvs_PART_SRCS
    cvsplugin.cpp
    cvsmainview.cpp
    cvsgenericoutputview.cpp
    cvsjob.cpp
    cvsproxy.cpp
    editorsview.cpp
    commitdialog.cpp
    importmetadatawidget.cpp
    importdialog.cpp
    checkoutdialog.cpp
    cvsannotatejob.cpp
    cvslogjob.cpp
    cvsdiffjob.cpp
    cvsstatusjob.cpp
)

set(kdevcvs_PART_UI
    cvsmainview.ui
    cvsgenericoutputview.ui
    editorsview.ui
    commitdialog.ui
    importmetadatawidget.ui
    checkoutdialog.ui
)

kde4_add_ui_files(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})
kde4_add_plugin(kdevcvs ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
    ${KDE4_KIO_LIBS}
    ${KDE4_KPARTS_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
    kdevplatformlanguage
)

install(TARGETS kdevcvs DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
configure_file(kdevcvs.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcvs.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcvs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcvs.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcvs )
