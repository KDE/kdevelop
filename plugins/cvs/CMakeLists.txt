
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
    KF5::KDELibs4Support
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KDev::Language
)

install(TARGETS kdevcvs DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############
configure_file(kdevcvs.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcvs.desktop)
kcoreaddons_desktop_to_json(kdevcvs ${CMAKE_CURRENT_BINARY_DIR}/kdevcvs.desktop)
install( FILES kdevcvs.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcvs )
