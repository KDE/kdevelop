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
add_library(kdevgrepview MODULE ${kdevgrepview_PART_SRCS})
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

install(TARGETS kdevgrepview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevgrepview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgrepview.desktop)
kcoreaddons_desktop_to_json(kdevgrepview ${CMAKE_CURRENT_BINARY_DIR}/kdevgrepview.desktop)
install( FILES kdevgrepview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevgrepview )

add_subdirectory(tests)
