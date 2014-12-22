add_definitions(-DTRANSLATION_DOMAIN=\"kdevcvs\")
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

ki18n_wrap_ui(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})

kdevplatform_add_plugin(kdevcvs JSON kdevcvs.json SOURCES ${kdevcvs_PART_SRCS})

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

########### install files ###############

install( FILES kdevcvs.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcvs )
