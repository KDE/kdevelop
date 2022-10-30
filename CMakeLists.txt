cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release service script
set(RELEASE_SERVICE_VERSION_MAJOR "22")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "3")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

# generate patch level from release service version
set(_micro ${RELEASE_SERVICE_VERSION_MICRO})
if (_micro LESS "10")
    string(PREPEND _micro "0") # pad with 0
endif()
set(RELEASE_SERVICE_BASED_PATCHLEVEL "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${_micro}")

project(KDevelop VERSION "5.9.${RELEASE_SERVICE_BASED_PATCHLEVEL}")

# KDevelop SOVERSION
# E.g. for KDevelop 5.2.0 => SOVERSION 52 (we only promise ABI compatibility between patch version updates)
set(KDEVELOP_SOVERSION "${KDevelop_VERSION_MAJOR}${KDevelop_VERSION_MINOR}")

# On macOS KDevelop_VERSION_MINOR is modified below. But KDEV_ITEMREPOSITORY_VERSION should depend
# on the original minor version on all platforms. So we store the original version in a variable.
set(KDEVELOP_OWN_MINOR_VERSION "${KDevelop_VERSION_MINOR}")

if (APPLE)
    ###################################################################################
    # Kludge for bug #448152: ld: malformed 64-bit a.b.c.d.e version number: 5.7.211201
    # Apple's linker uses the project version (not the SOVERSION) to calculate an
    # unsigned int in such a way that places restrictions on the magnitude of each tier
    # of the version string.
    # See here: https://www.sicpers.info/2013/03/how-to-version-a-mach-o-library/
    ###################################################################################
    project(KDevelop VERSION "${KDevelop_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${_micro}")
endif()

# plugin version as used e.g. in plugin installation path
# started off as 30 when 5.2 was released, incremented along the SO version so far
# TODO: perhaps switch directly to KDEVELOP_SOVERSION?
math(EXPR KDEV_PLUGIN_VERSION "30 + ${KDEVELOP_SOVERSION} - 52")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(QT_MIN_VERSION "5.15.0")
set(KF5_DEP_VERSION "5.78.0")

# we need some parts of the ECM CMake helpers
find_package (ECM ${KF5_DEP_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE) # needs to be first, as set policies influence following macros
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)
include(KDevelopMacrosInternal)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Concurrent Quick QuickWidgets)
if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()

find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
  Config
  Declarative
  DocTools
  IconThemes
  I18n
  ItemModels
  ItemViews
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  NotifyConfig
  Parts
  Service
  TextEditor
  ThreadWeaver
  XmlGui
  WindowSystem
  Crash
  GuiAddons
  Archive
  Notifications
)

set(HAVE_KSYSGUARD FALSE)
find_package(KSysGuard CONFIG QUIET)
set_package_properties(KSysGuard PROPERTIES
    PURPOSE "Framework for process listing. Required for the 'Attach to Process' feature"
    TYPE RECOMMENDED
)
if(NOT KSysGuard_FOUND)
    # try old pre-Plasma 5.19 name
    find_package(KF5SysGuard CONFIG)
    set_package_properties(KF5SysGuard PROPERTIES
        PURPOSE "Framework for process listing (older name of KSysGuard). Required for the 'Attach to Process' feature"
        TYPE RECOMMENDED
    )
endif()
if(KSysGuard_FOUND OR KF5SysGuard_FOUND)
    set(HAVE_KSYSGUARD TRUE)
endif()

find_package(KDevelop-PG-Qt 1.90.90 CONFIG)
set_package_properties(KDevelop-PG-Qt PROPERTIES
    PURPOSE "KDevelop parser generator library. Required for the QMake Builder/Manager plugin."
    TYPE RECOMMENDED
)

find_package(SharedMimeInfo 1.9 REQUIRED)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x053200
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
    -DQT_NO_FOREACH
)

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: https://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
add_compile_flag_if_supported(-Wno-missing-field-initializers)
add_compile_flag_if_supported(-Werror=switch)
add_compile_flag_if_supported(-Werror=undefined-bool-conversion)
add_compile_flag_if_supported(-Werror=tautological-undefined-compare)
add_compile_flag_if_supported(-Werror=implicit-fallthrough)    # Use Q_FALLTHROUGH for false positives.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_flag_if_supported(-Wdocumentation)
    add_compile_flag_if_supported(-Wcovered-switch-default)
    add_compile_flag_if_supported(-Wunreachable-code-break)
    # This warning is triggered by every call to qCDebug()
    add_compile_flag_if_supported(-Wno-gnu-zero-variadic-macro-arguments)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_flag_if_supported(-pedantic)
    add_compile_flag_if_supported(-Wzero-as-null-pointer-constant CXX_ONLY)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_flag_if_supported(-Wsuggest-override CXX_ONLY)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug"
  OR CMAKE_BUILD_TYPE_TOLOWER STREQUAL "")
  set(COMPILER_OPTIMIZATIONS_ENABLED OFF)
else()
  set(COMPILER_OPTIMIZATIONS_ENABLED ON)
endif()

option(BUILD_BENCHMARKS "Whether benchmarks should be compiled or not" ${COMPILER_OPTIMIZATIONS_ENABLED})

# create config-kdevelop.h
configure_file(config-kdevelop.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdevelop.h)
include_directories(${KDevelop_BINARY_DIR}) # for config-kdevelop.h

add_subdirectory(3rdparty/timsort)

add_subdirectory(kdevplatform)
add_subdirectory(plugins)

add_subdirectory(pics)
add_subdirectory(app)
add_subdirectory(app_templates)
add_subdirectory(file_templates)
add_subdirectory(shortcuts)
add_subdirectory(doc)
add_subdirectory(share)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDevelop")
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevelopConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfig.cmake"
                                  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KDEVELOP
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdevelop_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfigVersion.cmake"
)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kdevelop_version.h"
        DESTINATION "${KDE_INSTALL_INCLUDEDIR}/kdevelop")
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}" )
install(EXPORT KDevelopTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" NAMESPACE KDev:: FILE KDevelopTargets.cmake)

# kdebugsettings file
install_qt_logging_categories(TYPE APP_PLUGIN)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

install(FILES org.kde.kdevelop.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
