project(kdev-ninja)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevninja\")
set(kdevninja_SRCS ninjajob.cpp kdevninjabuilderplugin.cpp ninjabuilderpreferences.cpp)
ki18n_wrap_ui(kdevninja_SRCS ninjaconfig.ui)
kconfig_add_kcfg_files(kdevninja_SRCS ninjabuilderconfig.kcfgc)

add_library(kdevninja MODULE ${kdevninja_SRCS})
target_link_libraries(kdevninja
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Shell
    KDev::Util
    KDev::Shell
)

install(TARGETS kdevninja DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevninja.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop DESTINATION ${SERVICES_INSTALL_DIR})
