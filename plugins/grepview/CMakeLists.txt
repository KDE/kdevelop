add_definitions(-DTRANSLATION_DOMAIN=\"kdevgrepview\")
project(grepview)

########### next target ###############

set(kdevgrepview_PART_SRCS
    grepviewplugin.cpp
    grepviewpluginmetadata.cpp
    grepdialog.cpp
    grepoutputmodel.cpp
    grepoutputdelegate.cpp
    grepjob.cpp
    grepfindthread.cpp
    grepoutputview.cpp
    greputil.cpp
)

set(kdevgrepview_PART_UI
    grepwidget.ui
    grepoutputview.ui
)

ki18n_wrap_ui(kdevgrepview_PART_SRCS ${kdevgrepview_PART_UI})

kdevplatform_add_plugin(kdevgrepview JSON kdevgrepview.json SOURCES ${kdevgrepview_PART_SRCS})

target_link_libraries(kdevgrepview
    KF5::Parts
    KF5::TextEditor
    KF5::Completion
    KF5::TextEditor
    KF5::KDELibs4Support
    KDev::Interfaces
    KDev::OutputView
    KDev::Project
    KDev::Util
    KDev::Language
)

########### install files ###############

install( FILES kdevgrepview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevgrepview )

add_subdirectory(tests)
