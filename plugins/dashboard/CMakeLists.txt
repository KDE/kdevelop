project(kdev-project-dashboard)

set(kdevprojectdasboard_SRCS dashboardview.cpp dashboarddocument.cpp kdevprojectdashboard.cpp)

kde4_add_plugin(kdevprojectdashboard ${kdevprojectdasboard_SRCS})
target_link_libraries(kdevprojectdashboard ${KDE4_KDECORE_LIBS}
                        ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_SUBLIME_LIBRARIES})

install(TARGETS kdevprojectdashboard DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdevprojectdashboard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

