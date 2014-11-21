project(kdev-ninja)

add_library(kdevninja MODULE ninjajob.cpp kdevninjabuilderplugin.cpp)
target_link_libraries(kdevninja
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
)

install(TARGETS kdevninja DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### configurator ##############

ki18n_wrap_ui(ninjabuilder_cfg_SRCS ninjaconfig.ui)
kconfig_add_kcfg_files(ninjabuilder_cfg_SRCS ninjabuilderconfig.kcfgc)
add_library(kcm_kdev_ninjabuilder MODULE ninjabuilderpreferences.cpp ${ninjabuilder_cfg_SRCS})
target_link_libraries(kcm_kdev_ninjabuilder
	KF5::KIOWidgets
	KDev::Interfaces
	KDev::Project
	KDev::Util)

install(TARGETS kcm_kdev_ninjabuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)

configure_file(kcm_kdev_ninjabuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_ninjabuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_ninjabuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})

configure_file(kdevninja.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop DESTINATION ${SERVICES_INSTALL_DIR})

