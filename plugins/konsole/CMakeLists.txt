
########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleviewplugin.cpp
    kdevkonsoleview.cpp
)

add_library(kdevkonsoleview MODULE ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF5::Parts KF5::KDELibs4Support KDev::Interfaces)

install(TARGETS kdevkonsoleview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevkonsoleview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevkonsoleview.desktop)
kcoreaddons_desktop_to_json(kdevkonsoleview ${CMAKE_CURRENT_BINARY_DIR}/kdevkonsoleview.desktop)

