cmake_minimum_required(VERSION 3.0)

project(kdev-clangtidy)

find_package (ECM "5.14.0" REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED Core Widgets Test)
set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
    I18n
    ItemModels # needed because missing in KDevPlatformConfig.cmake, remove once dep on kdevplatform >=5.2.2
    Config
)
set(KDEVPLATFORM_DEP_VERSION "5.1.0")
find_package(KDevPlatform ${KDEVPLATFORM_DEP_VERSION} CONFIG)
set_package_properties(KDevPlatform PROPERTIES
    TYPE REQUIRED
)

find_package(Boost REQUIRED)

find_package(ClangTidy QUIET)
set_package_properties(ClangTidy PROPERTIES
    DESCRIPTION "A clang-based C++ “linter” tool"
    URL "http://clang.llvm.org/extra/clang-tidy/"
    TYPE RUNTIME
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050500
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

add_definitions(-DTRANSLATION_DOMAIN=\"kdevclangtidy\")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)

ecm_qt_declare_logging_category(kdevclangtidy_LOG_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_CLANGTIDY
    CATEGORY_NAME "kdevelop.analyzers.clangtidy"
)


set(kdevclangtidy_PART_SRCS
    src/job.cpp
    src/plugin.cpp

    src/config/configgroup.cpp
    src/config/perprojectconfigpage.cpp
    src/config/clangtidypreferences.cpp

    src/parsers/clangtidyparser.cpp
    src/parsers/replacementparser.cpp

    ${kdevclangtidy_LOG_SRCS}
)

include (cmake/ClangFormatAll.cmake)

ki18n_wrap_ui(kdevclangtidy_PART_SRCS ${kdevclangtidy_PART_UI}
    src/config/clangtidypreferences.ui
    src/config/perprojectconfig.ui
)

qt5_add_resources(kdevclangtidy_PART_SRCS
    kdevclangtidy.qrc
)

kconfig_add_kcfg_files(kdevclangtidy_PART_SRCS src/config/clangtidyconfig.kcfgc)
kdevplatform_add_plugin(kdevclangtidy JSON kdevclangtidy.json SOURCES
${kdevclangtidy_PART_SRCS} ${kdevclangtidy_CONFIG_SRCS})
target_link_libraries(kdevclangtidy
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
    KDev::Shell
    KF5::ConfigCore
    KF5::I18n
)

add_subdirectory(tests)

# kdebugsettings file
install(FILES kdevclangtidy.categories DESTINATION ${KDE_INSTALL_CONFDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
