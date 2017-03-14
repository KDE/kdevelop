cmake_minimum_required(VERSION 2.8.12)

# kdevplatform version
set(KDEVPLATFORM_VERSION_MAJOR 5)
set(KDEVPLATFORM_VERSION_MINOR 1)
set(KDEVPLATFORM_VERSION_PATCH 40)

# plugin versions listed in the .desktop files
set(KDEV_PLUGIN_VERSION 27)
# Increase this to reset incompatible item-repositories
set(KDEV_ITEMREPOSITORY_VERSION 86)

# library version / SO version
set(KDEVPLATFORM_LIB_VERSION 10.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 10)

project(KDevPlatform)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDECompilerSettings NO_POLICY_SCOPE)
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
include(KDevPlatformMacros)

set(QT_MIN_VERSION "5.4.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Concurrent Test)

find_package(Qt5QuickWidgets ${QT_MIN_VERSION})
set_package_properties(Qt5QuickWidgets PROPERTIES
    PURPOSE "Qt5 QuickWidgets library (part of Qt >=5.3). Required for the Welcome Page plugin."
)

set(KF5_DEP_VERSION "5.18.0") # need KAboutData::fromPluginMetaData in kcoreaddons
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
  Archive
  Config
  GuiAddons
  WidgetsAddons
  IconThemes
  I18n
  ItemModels
  ItemViews
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  Notifications
  NotifyConfig
  Parts
  Service
  Sonnet
  TextEditor
  ThreadWeaver
  WindowSystem
  Declarative
  XmlGui
)

find_package(Grantlee5)
set_package_properties(Grantlee5 PROPERTIES
    PURPOSE "Grantlee templating library, needed for file templates"
    URL "http://www.grantlee.org/"
    TYPE RECOMMENDED)

set(Boost_ADDITIONAL_VERSIONS 1.39.0 1.39)
find_package(Boost 1.35.0)
set_package_properties(Boost PROPERTIES
    PURPOSE "Boost libraries for enabling the classbrowser"
    URL "http://www.boost.org"
    TYPE REQUIRED)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
check_cxx_compiler_flag(-Wno-missing-field-initializers HAVE_MFI_FLAG)
check_cxx_compiler_flag(-Werror=undefined-bool-conversion HAVE_UBC_FLAG)
check_cxx_compiler_flag(-Werror=tautological-undefined-compare HAVE_TUC_FLAG)
if (HAVE_MFI_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()
if (HAVE_UBC_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=undefined-bool-conversion")
endif()
if (HAVE_TUC_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=tautological-undefined-compare")
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdocumentation")
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-kdevplatform.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-kdevplatform.h )

include_directories(${KDevPlatform_SOURCE_DIR} ${KDevPlatform_BINARY_DIR})

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
add_subdirectory(debugger)
add_subdirectory(documentation)
add_subdirectory(serialization)
add_subdirectory(template)
add_subdirectory(tests)
add_subdirectory(plugins)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDevPlatform")

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
        DESTINATION "${KDE_INSTALL_INCLUDEDIR}/kdevplatform" )
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
