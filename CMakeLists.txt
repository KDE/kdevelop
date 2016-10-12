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
include(FeatureSummary)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS IconThemes ItemModels ThreadWeaver TextEditor I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include_directories(
    ${cppcheck_BINARY_DIR}
    ${cppcheck_SOURCE_DIR}
    ${cppcheck_SOURCE_DIR}/config
)

set(kdevcppcheck_SRCS
    debug.cpp
    job.cpp
    parser.cpp
    plugin.cpp

    config/globalconfigpage.cpp
    config/genericconfigpage.cpp
)

ki18n_wrap_ui(kdevcppcheck_UI_SRCS
    config/globalconfigpage.ui
    config/ui/genericconfig.ui
)

kconfig_add_kcfg_files(kdevcppcheck_CONFIG_SRCS
    config/globalsettings.kcfgc
)

kdevplatform_add_plugin(kdevcppcheck
    JSON kdevcppcheck.json
    SOURCES ${kdevcppcheck_SRCS} ${kdevcppcheck_UI_SRCS} ${kdevcppcheck_CONFIG_SRCS}
)

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

add_subdirectory(tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

