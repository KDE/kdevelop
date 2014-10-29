add_subdirectory(tests)

set(kdevkdeprovider_PART_SRCS kdeprojectsmodel.cpp
    kdeproviderwidget.cpp
    kdeproviderplugin.cpp
    kdeprojectsmodel.cpp
    kdeprojectsreader.cpp
)
kconfig_add_kcfg_files(kdevkdeprovider_PART_SRCS kdeconfig.kcfgc)
qt5_wrap_ui(kdevkdeprovider_PART_SRCS kdeconfig.ui)

add_library(kdevkdeprovider MODULE ${kdevkdeprovider_PART_SRCS})
target_link_libraries(kdevkdeprovider KF5::ItemViews KDev::Interfaces KF5::KDELibs4Support KDev::Vcs)

install(TARGETS kdevkdeprovider DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevkdeprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevkdeprovider.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevkdeprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})
