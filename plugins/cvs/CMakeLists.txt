
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

qt5_wrap_ui(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})
add_library(kdevcvs MODULE ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
    KF5::KIOWidgets
    KF5::Parts
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
install( FILES kdevcvs.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcvs )
