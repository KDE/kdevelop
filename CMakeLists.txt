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

if(BUILD_TESTING)
    set(QT_ADDITIONAL_REQ_COMPONENTS Test)
endif()
set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets ${QT_ADDITIONAL_REQ_COMPONENTS})

set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
    I18n
    ItemViews
    ItemModels # needed because missing in KDevPlatformConfig.cmake, remove once dep on kdevplatform >=5.2.2
    Config
)
set(KDEVPLATFORM_DEP_VERSION "5.2.0")
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
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

include (cmake/ClangFormatAll.cmake)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

ecm_qt_declare_logging_category(kdevclangtidy_LOG_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_CLANGTIDY
    CATEGORY_NAME "kdevelop.analyzers.clangtidy"
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# kdebugsettings file
install(FILES kdevclangtidy.categories DESTINATION ${KDE_INSTALL_CONFDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
