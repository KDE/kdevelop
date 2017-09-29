cmake_minimum_required(VERSION 3.0)
project(KDevelop VERSION 5.1.80)

# KDevelop SOVERSION
# E.g. for KDevelop 5.2.0 => SOVERSION 52 (we only promise ABI compatibility between patch version updates)
set(KDEVELOP_SOVERSION 52)

# plugin versions listed in the .desktop files
set(KDEV_PLUGIN_VERSION 30)

# we need some parts of the ECM CMake helpers
find_package (ECM 5.14.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

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

include(KDEInstallDirs)
include(KDECMakeSettings)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Concurrent Quick QuickWidgets)
if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()
set(KF5_DEP_VERSION "5.15.0") # we need KCrash::initialize
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
find_package(KF5SysGuard CONFIG)
set_package_properties(KF5SysGuard PROPERTIES
    PURPOSE "Framework for process listing. Required for the 'Attach to Process' feature"
    TYPE RECOMMENDED
)
find_package(KDevelop-PG-Qt 1.90.90 CONFIG)
set_package_properties(KDevelop-PG-Qt PROPERTIES
    PURPOSE "KDevelop parser generator library. Required for the QMake Builder/Manager plugin."
    TYPE RECOMMENDED
)

find_package(SharedMimeInfo REQUIRED)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050500
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
)

function(add_compile_flag_if_supported _flag)
    unset(_have_flag CACHE)
    string(REGEX REPLACE "[-=]" "_" _varname ${_flag})
    string(TOUPPER ${_varname} _varname)
    set(_varname "HAVE${_varname}")
    check_cxx_compiler_flag("${_flag}" "${_varname}")
    if (${${_varname}})
        add_compile_options(${_flag})
    endif()
endfunction()

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
add_compile_flag_if_supported(-Wno-missing-field-initializers)
add_compile_flag_if_supported(-Werror=undefined-bool-conversion)
add_compile_flag_if_supported(-Werror=tautological-undefined-compare)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_flag_if_supported(-Wdocumentation)
    # This warning is triggered by every call to qCDebug()
    add_compile_flag_if_supported(-Wno-gnu-zero-variadic-macro-arguments)
endif()
if (CMAKE_COMPILER_CXX_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_flag_if_supported(-pedantic)
endif()

include_directories(${KDevelop_SOURCE_DIR} ${KDevelop_BINARY_DIR})

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug"
  OR CMAKE_BUILD_TYPE_TOLOWER STREQUAL "")
  set(COMPILER_OPTIMIZATIONS_DISABLED TRUE)
else()
  set(COMPILER_OPTIMIZATIONS_DISABLED FALSE)
endif()

# create config-kdevelop.h
configure_file(config-kdevelop.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdevelop.h)

add_subdirectory(kdevplatform)
add_subdirectory(plugins)

add_subdirectory(pics)
add_subdirectory(app)
add_subdirectory(app_templates)
add_subdirectory(file_templates)
add_subdirectory(shortcuts)
add_subdirectory(doc)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDevelop")
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevelopConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfig.cmake"
                                  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
ecm_setup_version(${KDEVELOP_VERSION_MAJOR}.${KDEVELOP_VERSION_MINOR}.${KDEVELOP_VERSION_PATCH}
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
install(FILES kdevelop.categories DESTINATION ${KDE_INSTALL_CONFDIR})

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

install(FILES org.kde.kdevelop.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install("${CMAKE_CURRENT_BINARY_DIR}/po")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
