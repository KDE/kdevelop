project(cppcheck)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set(KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

cmake_minimum_required(VERSION 2.8.12)
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS IconThemes ItemModels ThreadWeaver TextEditor I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include_directories(
    ${cppcheck_BINARY_DIR}
    ${cppcheck_SOURCE_DIR}
    ${cppcheck_SOURCE_DIR}/config
    ${cppcheck_SOURCE_DIR}/parsers
)

set(kdevcppcheck_PART_SRCS
    debug.cpp
    job.cpp
    iparser.cpp
    plugin.cpp
    config/genericconfigpage.cpp
    config/cppcheckpreferences.cpp
    parsers/cppcheckparser.cpp
)

ki18n_wrap_ui(kdevcppcheck_PART_SRCS  ${kdevcppcheck_PART_UI} config/ui/genericconfig.ui config/ui/cppchecksettings.ui)
kconfig_add_kcfg_files(kdevcppcheck_PART_SRCS config/cppcheckconfig.kcfgc)
kdevplatform_add_plugin(kdevcppcheck JSON kdevcppcheck.json SOURCES ${kdevcppcheck_PART_SRCS} ${kdevcppcheck_CONFIG_SRCS})
target_link_libraries(kdevcppcheck
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
    KDev::Shell
    KF5::ConfigCore
    KF5::IconThemes
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::TextEditor
    Qt5::Network)

install(FILES kdevcppcheck.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppcheck)
