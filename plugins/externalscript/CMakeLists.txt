add_definitions(-DTRANSLATION_DOMAIN=\"kdevexternalscript\")
########### next target ###############

set(kdevexternalscript_PART_SRCS
    externalscriptplugin.cpp
    externalscriptview.cpp
    externalscriptitem.cpp
    externalscriptjob.cpp
    editexternalscript.cpp
)

declare_qt_logging_category(kdevexternalscript_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_EXTERNALSCRIPT
    CATEGORY_BASENAME "externalscript"
)

set(kdevexternalscript_PART_UI
    externalscriptview.ui
    editexternalscript.ui
)

ki18n_wrap_ui(kdevexternalscript_PART_SRCS ${kdevexternalscript_PART_UI})

qt_add_resources(kdevexternalscript_PART_SRCS kdevexternalscript.qrc)
kdevplatform_add_plugin(kdevexternalscript SOURCES ${kdevexternalscript_PART_SRCS})

target_link_libraries(kdevexternalscript
    KDev::Language
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Util
    KDev::OutputView
    KF6::TextEditor
    KF6::KIOWidgets
    KF6::Parts
)
