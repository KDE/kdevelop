project(kdev-ninja)

set(kdevninja_SRCS ninjajob.cpp kdevninjabuilderplugin.cpp ninjabuilderpreferences.cpp)
ki18n_wrap_ui(kdevninja_SRCS ninjaconfig.ui)
kconfig_add_kcfg_files(kdevninja_SRCS ninjabuilderconfig.kcfgc)

add_library(kdevninja MODULE ${kdevninja_SRCS})
target_link_libraries(kdevninja
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
    KDev::Shell
)

install(TARGETS kdevninja DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
