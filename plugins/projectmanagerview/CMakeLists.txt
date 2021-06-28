add_definitions(-DTRANSLATION_DOMAIN=\"kdevprojectmanagerview\")
########### next target ###############

set(kdevprojectmanagerview_PLUGIN_SRCS
    projectmanagerviewplugin.cpp
    projectmanagerview.cpp
    projectmodelsaver.cpp
    projecttreeview.cpp
    projectbuildsetwidget.cpp
    vcsoverlayproxymodel.cpp
    projectmodelitemdelegate.cpp
    cutcopypastehelpers.cpp
)

declare_qt_logging_category(kdevprojectmanagerview_PLUGIN_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_PROJECTMANAGERVIEW
    CATEGORY_BASENAME "projectmanagerview"
)
ki18n_wrap_ui( kdevprojectmanagerview_PLUGIN_SRCS projectbuildsetwidget.ui projectmanagerview.ui )

qt5_add_resources(kdevprojectmanagerview_PLUGIN_SRCS kdevprojectmanagerview.qrc)
kdevplatform_add_plugin(kdevprojectmanagerview SOURCES ${kdevprojectmanagerview_PLUGIN_SRCS})

target_link_libraries(kdevprojectmanagerview
    KF5::TextEditor
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Util
    KDev::Vcs
    KDev::Sublime
)



