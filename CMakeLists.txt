project(clangtidy)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
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
    ${clangtidy_BINARY_DIR}
    ${clangtidy_SOURCE_DIR}/src
    ${clangtidy_SOURCE_DIR}/src/config
    ${clangtidy_SOURCE_DIR}/src/parsers
)

set(kdevclangtidy_PART_SRCS
    src/debug.cpp
    src/job.cpp
    src/plugin.cpp
    src/config/configgroup.cpp
    src/config/genericconfigpage.cpp
    src/config/clangtidypreferences.cpp
    src/parsers/clangtidyparser.cpp
)

ki18n_wrap_ui(kdevclangtidy_PART_SRCS  ${kdevclangtidy_PART_UI} 
src/config/ui/genericconfig.ui src/config/ui/clangtidysettings.ui)
kconfig_add_kcfg_files(kdevclangtidy_PART_SRCS src/config/clangtidyconfig.kcfgc)
kdevplatform_add_plugin(kdevclangtidy JSON res/kdevclangtidy.json SOURCES 
${kdevclangtidy_PART_SRCS} ${kdevclangtidy_CONFIG_SRCS})
target_link_libraries(kdevclangtidy
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

install(FILES res/kdevclangtidy.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevclangtidy)

# add_subdirectory(tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

