# kdevplatform version
set(KDEVPLATFORM_VERSION_MAJOR 1)
set(KDEVPLATFORM_VERSION_MINOR 90)
set(KDEVPLATFORM_VERSION_PATCH 90)

# plugin versions listed in the .desktop files
set(KDEV_PLUGIN_VERSION 20)
# Increase this to reset incompatible item-repositories
set(KDEV_ITEMREPOSITORY_VERSION 81)

# library version / SO version
set(KDEVPLATFORM_LIB_VERSION 9.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 9)

################################################################################

cmake_minimum_required(VERSION 2.8.12)
project(KDevPlatform)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMAddTests)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(GenerateExportHeader)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(KDevPlatformMacros)

set(QT_MIN_VERSION "5.2.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Declarative DBus Widgets Script WebKitWidgets Concurrent Test)

find_package(Qt5QuickWidgets ${QT_MIN_VERSION})
set_package_properties(Qt5QuickWidgets PROPERTIES
    PURPOSE "Qt5 QuickWidgets library (part of Qt >=5.3). Required for the Welcome Page plugin."
)

find_package(KF5 REQUIRED COMPONENTS
  Archive
  Config
  GuiAddons
  WidgetsAddons
  I18n
  ItemModels
  ItemViews
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  NotifyConfig
  Parts
  Sonnet
  TextEditor
  ThreadWeaver
  WindowSystem
  Declarative

  # TODO KF5: Port away from this
  KDELibs4Support
)

find_package(KF5XmlGui 5.1 REQUIRED)
find_package(KF5Service 5.3 REQUIRED)

find_package(Grantlee5 REQUIRED)
set_package_properties(Grantlee5 PROPERTIES
    PURPOSE "Grantlee templating library, needed for file templates"
    URL "http://www.grantlee.org/")

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0) # comes from KDELibs4Support
add_definitions(-DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x050000)

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-kdevplatform.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-kdevplatform.h )

include_directories(${KDevPlatform_SOURCE_DIR} ${KDevPlatform_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/util/)

# Now set the usual KDEVPLATFORM_XXX_LIBRARIES variable so we can more easily move plugins around
set(KDEVPLATFORM_SUBLIME_LIBRARIES KDev::Sublime)
set(KDEVPLATFORM_INTERFACES_LIBRARIES KDev::Interfaces)
set(KDEVPLATFORM_LANGUAGE_LIBRARIES KDev::Language)
set(KDEVPLATFORM_PROJECT_LIBRARIES KDev::Project)
set(KDEVPLATFORM_UTIL_LIBRARIES KDev::Util)
set(KDEVPLATFORM_OUTPUTVIEW_LIBRARIES KDev::OutputView)
set(KDEVPLATFORM_VCS_LIBRARIES KDev::Vcs)
set(KDEVPLATFORM_SHELL_LIBRARIES KDev::Shell)
set(KDEVPLATFORM_TESTS_LIBRARIES KDev::Tests)
set(KDEVPLATFORM_JSONTESTS_LIBRARIES KDev::JsonTests)
set(KDEVPLATFORM_DEBUGGER_LIBRARIES KDev::Debugger)
set(KDEVPLATFORM_DOCUMENTATION_LIBRARIES KDev::Documentation)
set(KDEVPLATFORM_SERIALIZATION_LIBRARIES KDev::Serialization)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug"
  OR CMAKE_BUILD_TYPE_TOLOWER STREQUAL "")
  set(COMPILER_OPTIMIZATIONS_DISABLED TRUE)
else()
  set(COMPILER_OPTIMIZATIONS_DISABLED FALSE)
endif()

add_subdirectory(sublime)
add_subdirectory(interfaces)
add_subdirectory(project)
add_subdirectory(language)
add_subdirectory(shell)
add_subdirectory(util)
add_subdirectory(outputview)
add_subdirectory(vcs)
add_subdirectory(pics)
#ecm_optional_add_subdirectory(doc)
add_subdirectory(debugger)
add_subdirectory(documentation)
add_subdirectory(serialization)

add_subdirectory(template)
add_subdirectory(tests)

add_subdirectory(plugins)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDevPlatform")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevPlatformConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDevPlatformConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

ecm_setup_version(${KDEVPLATFORM_VERSION_MAJOR}.${KDEVPLATFORM_VERSION_MINOR}.${KDEVPLATFORM_VERSION_PATCH}
                  VARIABLE_PREFIX KDEVPLATFORM
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdevplatform_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
                  SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

install( FILES
        "${KDevPlatform_BINARY_DIR}/kdevplatform_version.h"
        "${KDevPlatform_BINARY_DIR}/config-kdevplatform.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake"
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
        cmake/modules/KDevPlatformMacros.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}" )
install( EXPORT KDevPlatformTargets
         DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
         NAMESPACE KDev::
         FILE KDevPlatformTargets.cmake )

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
