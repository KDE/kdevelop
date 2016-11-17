project(kdevcppcheck)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set(KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

cmake_minimum_required(VERSION 2.8.12)

find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS ItemModels ItemViews)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include_directories(
    ${kdevcppcheck_BINARY_DIR}
    ${kdevcppcheck_SOURCE_DIR}
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)

set(kdevcppcheck_SRCS
    plugin.cpp

    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)

ki18n_wrap_ui(kdevcppcheck_UI_SRCS
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)

kconfig_add_kcfg_files(kdevcppcheck_CONFIG_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)

add_library(kdevcppcheck_core STATIC
    debug.cpp
    parser.cpp
    job.cpp
    parameters.cpp

    ${kdevcppcheck_CONFIG_SRCS}
)

target_link_libraries(kdevcppcheck_core
    KDev::Language
    KDev::Project
    KDev::Shell
)

kdevplatform_add_plugin(kdevcppcheck
    JSON kdevcppcheck.json
    SOURCES ${kdevcppcheck_SRCS} ${kdevcppcheck_UI_SRCS}
)

target_link_libraries(kdevcppcheck
    kdevcppcheck_core
    KF5::ItemViews
)

install(FILES kdevcppcheck.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppcheck)

add_subdirectory(tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
