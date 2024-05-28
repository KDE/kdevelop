add_definitions(-DTRANSLATION_DOMAIN=\"kdevgrepview\")

########### next target ###############

declare_qt_logging_category(kdevgrepview_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_GREPVIEW
    CATEGORY_BASENAME "grepview"
)

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
    ${kdevgrepview_LOG_PART_SRCS}
)

set(kdevgrepview_PART_UI
    grepwidget.ui
    grepoutputview.ui
)

ki18n_wrap_ui(kdevgrepview_PART_SRCS ${kdevgrepview_PART_UI})

qt_add_resources(kdevgrepview_PART_SRCS kdevgrepview.qrc)
kdevplatform_add_plugin(kdevgrepview SOURCES ${kdevgrepview_PART_SRCS})

target_link_libraries(kdevgrepview
    KF6::Parts
    KF6::TextEditor
    KF6::Completion
    KF6::TextEditor
    KF6::Codecs
    KF6::KIOWidgets
    KDev::Interfaces
    KDev::OutputView
    KDev::Project
    KDev::Util
    KDev::Language
    Qt::DBus
)

########### install files ###############

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
