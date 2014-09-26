project(vcschangesview)

set(vcschangesview_SRCS
    vcschangesview.cpp
    
    vcschangesviewplugin.cpp
)

add_library(kdevvcschangesviewplugin MODULE ${vcschangesview_SRCS})
target_link_libraries(kdevvcschangesviewplugin KDev::Interfaces KDev::Project KDev::Vcs)

install(TARGETS kdevvcschangesviewplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

configure_file(kdevvcschangesview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevvcschangesview.desktop)
kcoreaddons_desktop_to_json(kdevvcschangesviewplugin ${CMAKE_CURRENT_BINARY_DIR}/kdevvcschangesview.desktop)
