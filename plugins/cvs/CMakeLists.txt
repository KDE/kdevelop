add_definitions(-DTRANSLATION_DOMAIN=\"kdevcvs\")
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


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

qt5_add_resources(kdevcvs_PART_SRCS kdevcvs.qrc)
kdevplatform_add_plugin(kdevcvs JSON kdevcvs.json SOURCES ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
    KF5::KIOWidgets
    KF5::Parts
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KDev::Language
)
