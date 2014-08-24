
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9511 )

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

qt5_wrap_ui( kdevprojectmanagerview_PLUGIN_SRCS projectbuildsetwidget.ui projectmanagerview.ui )

add_library(kdevprojectmanagerview MODULE ${kdevprojectmanagerview_PLUGIN_SRCS})
target_link_libraries(kdevprojectmanagerview KF5::TextEditor
                      KDev::Interfaces KDev::Project KDev::Language KDev::Util KDev::Vcs KDev::Sublime KF5::ThreadWeaver)

install(TARGETS kdevprojectmanagerview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevprojectmanagerview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectmanagerview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectmanagerview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanagerview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevprojectmanagerview )


