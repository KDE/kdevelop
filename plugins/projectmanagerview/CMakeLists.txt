
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9511 )

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${THREADWEAVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
    )


########### next target ###############

set(kdevprojectmanagerview_PLUGIN_SRCS
    projectmanagerviewplugin.cpp
    projectmanagerview.cpp
    projecttreeview.cpp
    projectbuildsetwidget.cpp
    projectbuildsetmodel.cpp
    projectproxymodel.cpp
)

kde4_add_ui_files( kdevprojectmanagerview_PLUGIN_SRCS projectbuildsetwidget.ui )

kde4_add_plugin(kdevprojectmanagerview ${kdevprojectmanagerview_PLUGIN_SRCS})
target_link_libraries(kdevprojectmanagerview ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject ${KDE4_THREADWEAVER_LIBRARIES} )

install(TARGETS kdevprojectmanagerview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevprojectmanagerview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanagerview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanagerview )


