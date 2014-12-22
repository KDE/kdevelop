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
)

ki18n_wrap_ui( kdevprojectmanagerview_PLUGIN_SRCS projectbuildsetwidget.ui projectmanagerview.ui )

kdevplatform_add_plugin(kdevprojectmanagerview JSON kdevprojectmanagerview.json SOURCES ${kdevprojectmanagerview_PLUGIN_SRCS})

target_link_libraries(kdevprojectmanagerview
    KF5::TextEditor
    KF5::ThreadWeaver
    KF5::KDELibs4Support
    KDev::Interfaces KDev::Project KDev::Language KDev::Util KDev::Vcs KDev::Sublime )

########### install files ###############

install( FILES kdevprojectmanagerview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevprojectmanagerview )


