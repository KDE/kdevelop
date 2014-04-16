project(projectitemelement)

set(projectfileelement_SRCS projectfileelement.cpp)

qt5_wrap_ui(projectfileelement_SRCS config.ui)
add_library(plasma_kdev_projectfileelement MODULE ${projectfileelement_SRCS})
target_link_libraries(plasma_kdev_projectfileelement KF5::Plasma KF5::KDELibs4Support)

install(TARGETS plasma_kdev_projectfileelement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdev-dash-projectfileelement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
