# kdevplatform version
set(KDEVPLATFORM_VERSION_MAJOR 1)
set(KDEVPLATFORM_VERSION_MINOR 90)
set(KDEVPLATFORM_VERSION_PATCH 60)

# plugin versions listed in the .desktop files
set(KDEV_PLUGIN_VERSION 19)

# library version / SO version
set(KDEVPLATFORM_LIB_VERSION 8.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 8)

################################################################################

cmake_minimum_required(VERSION 2.8.12)
project(KDevPlatform)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(KDevPlatformMacros)

set(QT_MIN_VERSION "5.2.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Declarative DBus Widgets Script WebKitWidgets Concurrent Test)

find_package(KF5 CONFIG REQUIRED COMPONENTS
  Archive
  Config
  DocTools
  GuiAddons
  I18n
  Init
  ItemModels
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  Parts
  PrintUtils
  Sonnet
  TextEditor
  ThreadWeaver
  WindowSystem
  XmlGui
  TextEditor
  NotifyConfig
  ThreadWeaver
  ItemModels
  Service
  Emoticons
  KCMUtils
  ItemViews

  # TODO KF5: Port away from this
  KDE4Support
)

#TODO KF5: Re-enable Grantlee (currently GrantleeConfig.cmake calls find_package(Qt4) internall
#find_package(Grantlee 0.1.7)
# set_package_properties(Grantlee PROPERTIES
#     PURPOSE "Grantlee templating library, needed for file templates"
#     URL "http://www.grantlee.org/"
#     TYPE REQUIRED)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQURL_NO_CAST_FROM_QSTRING)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # TODO: kf5 remove
    # in Qt 4, clang support is lacking...
    add_definitions("-DQ_COMPILER_INITIALIZER_LISTS=1")
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-kdevplatform.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-kdevplatform.h )

include_directories(${KDevPlatform_SOURCE_DIR} ${KDevPlatform_BINARY_DIR})
# include_directories(${Grantlee_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/util/)

# Now set the usual KDEVPLATFORM_XXX_LIBRARIES variable so we can more easily move plugins around
set(KDEVPLATFORM_SUBLIME_LIBRARIES sublime)
set(KDEVPLATFORM_INTERFACES_LIBRARIES kdevplatforminterfaces)
set(KDEVPLATFORM_LANGUAGE_LIBRARIES kdevplatformlanguage)
set(KDEVPLATFORM_PROJECT_LIBRARIES kdevplatformproject)
set(KDEVPLATFORM_UTIL_LIBRARIES kdevplatformutil)
set(KDEVPLATFORM_OUTPUTVIEW_LIBRARIES kdevplatformoutputview)
set(KDEVPLATFORM_VCS_LIBRARIES kdevplatformvcs)
set(KDEVPLATFORM_SHELL_LIBRARIES kdevplatformshell)
set(KDEVPLATFORM_TESTS_LIBRARIES kdevplatformtests)
set(KDEVPLATFORM_JSONTESTS_LIBRARIES kdevplatformjsontests)
set(KDEVPLATFORM_DEBUGGER_LIBRARIES kdevplatformdebugger)
set(KDEVPLATFORM_DOCUMENTATION_LIBRARIES kdevplatformdocumentation)

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

# if (Grantlee_FOUND)
#     add_subdirectory(template)
# endif()
add_subdirectory(tests)

add_subdirectory(plugins)

write_basic_config_version_file(${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake
    VERSION ${KDEVPLATFORM_VERSION_MAJOR}.${KDEVPLATFORM_VERSION_MINOR}.${KDEVPLATFORM_VERSION_PATCH}
    COMPATIBILITY AnyNewerVersion)
configure_file( "${KDevPlatform_SOURCE_DIR}/KDevPlatformConfig.cmake.in" "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake" @ONLY )
configure_file( "${KDevPlatform_SOURCE_DIR}/kdevplatformversion.h.cmake" "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h" @ONLY )

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDevPlatform")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevPlatformConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDevPlatformConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install( FILES
        "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/config-kdevplatform.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake"
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
        cmake/modules/KDevPlatformMacros.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}" )
install( EXPORT KDevPlatformTargets
         DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
         NAMESPACE KDevPlatformImport__
         FILE KDevPlatformTargets.cmake )

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
