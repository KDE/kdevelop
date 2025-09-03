cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release service script
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "1")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

# generate patch level from release service version
set(_micro ${RELEASE_SERVICE_VERSION_MICRO})
if (_micro LESS "10")
    string(PREPEND _micro "0") # pad with 0
endif()
set(RELEASE_SERVICE_BASED_PATCHLEVEL "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${_micro}")

# The math expression below automatically increments KDevelop minor version in each KDE Gear release.
# The multiplier 3 is the number of releases that share ${RELEASE_SERVICE_VERSION_MAJOR}.
# The fraction assigns consecutive minor versions to the 3 releases - 04, 08 and 12. The rounding up groups
# pre-release versions with the corresponding final release versions: 03 with 04, 07 with 08 and 11 with 12.
# The final subtrahend at the end of the expression ensures version continuity: KDevelop 6.0 in KDE Gear 24.08.
math(EXPR KDEVELOP_OWN_MINOR_VERSION "${RELEASE_SERVICE_VERSION_MAJOR} * 3                  \
                                      + (${RELEASE_SERVICE_VERSION_MINOR} + 4 - 3) / 4      \
                                      - 74")

# On macOS KDevelop_VERSION_MINOR is modified below. But KDEV_ITEMREPOSITORY_VERSION should depend on the
# original minor version on all platforms. So the variable ${KDEVELOP_OWN_MINOR_VERSION} is used again later.
project(KDevelop VERSION "6.${KDEVELOP_OWN_MINOR_VERSION}.${RELEASE_SERVICE_BASED_PATCHLEVEL}")

# KDevelop SOVERSION
# E.g. for KDevelop 5.2.0 => SOVERSION 52 (we only promise ABI compatibility between patch version updates)
set(KDEVELOP_SOVERSION "${KDevelop_VERSION_MAJOR}${KDevelop_VERSION_MINOR}")

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
set(KDEV_PLUGIN_VERSION "${KDEVELOP_SOVERSION}")

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

option(BUILD_DOC "Whether to build the documentation" ON)

# we need some parts of the ECM CMake helpers
find_package (ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
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
include(ECMDeprecationSettings)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)
include(KDevelopMacrosInternal)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Concurrent
    Core5Compat
    DBus
    Quick
    QuickWidgets
    Widgets
)
if(BUILD_TESTING)
  find_package(Qt6Test ${QT_MIN_VERSION} REQUIRED)
endif()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
  Config
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
  Sonnet
  TextTemplate
  TextWidgets
)
if(BUILD_DOC)
    find_package(KF6DocTools ${KF_MIN_VERSION} REQUIRED)
    set_package_properties(KF6DocTools PROPERTIES
        DESCRIPTION "Tools to generate documentation"
        TYPE REQUIRED
    )
endif()

set(HAVE_KSYSGUARD FALSE)
find_package(KSysGuard CONFIG QUIET)
set_package_properties(KSysGuard PROPERTIES
    PURPOSE "Framework for process listing. Required for the 'Attach to Process' feature"
    TYPE RECOMMENDED
)
if(KSysGuard_FOUND)
    set(HAVE_KSYSGUARD TRUE)
endif()

find_package(KDevelopPGQt 2.4 QUIET) # TODO (once we require KDevelopPGQt version 2.4 or later): remove QUIET again
set_package_properties(KDevelopPGQt PROPERTIES
    PURPOSE "KDevelop parser generator library. Required for the QMake Builder/Manager plugin."
    TYPE RECOMMENDED
)
if (NOT KDevelopPGQt_FOUND)
    find_package(KDevelop-PG-Qt 2.3)
    set_package_properties(KDevelop-PG-Qt PROPERTIES
        PURPOSE "KDevelop parser generator library (older name & version). Required for the QMake Builder/Manager plugin."
        TYPE RECOMMENDED
    )
endif()

find_package(SharedMimeInfo 1.9 REQUIRED)

# added by KDE_COMPILERSETTINGS_LEVEL 5.85.0
remove_definitions(
    -DQT_NO_KEYWORDS
)
add_definitions(
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)
ecm_set_disabled_deprecation_versions(
    DISABLE_NEWER_WARNINGS # remove again once Qt6/KF6 porting work fully done
    QT ${QT_MIN_VERSION}
    KF ${KF_MIN_VERSION}
)

# Globally enable PIC since static libraries need it for Qt staticMetaObject
# Furthermore, without that we cannot easily compile with ASAN either
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: https://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
add_compile_flag_if_supported(-Wno-missing-field-initializers)
add_compile_flag_if_supported(-Werror=switch)
add_compile_flag_if_supported(-Werror=undefined-bool-conversion)
add_compile_flag_if_supported(-Werror=tautological-undefined-compare)
add_compile_flag_if_supported(-Werror=implicit-fallthrough)    # Use [[fallthrough]] for false positives.
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
add_subdirectory(share)

if(BUILD_DOC)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

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

install(FILES org.kde.kdevelop.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
