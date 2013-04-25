cmake_minimum_required(VERSION 2.6)

project(KDevPlatform)

set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules)

set(KDEVPLATFORM_VERSION_MAJOR 1)
set(KDEVPLATFORM_VERSION_MINOR 5)
set(KDEVPLATFORM_VERSION_PATCH 0)

set(KDE_MIN_VERSION "4.5.0")
find_package(KDE4 4.5.0 REQUIRED)
set(KDEVPLATFORM_LIB_VERSION 7.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 7)

find_package(ZLIB REQUIRED)
find_package(Grantlee 0.1.7 REQUIRED)
find_package(QJSON)
macro_log_feature( QJSON_FOUND "QJson" "QJson support for Uploading Patches to reviewboard" "http://qjson.sourceforge.net/" FALSE "0.7.0" "The qjson library is needed to enable uploading patches to reviewboard" )

include (KDE4Defaults)
include (MacroWriteBasicCMakeVersionFile)
include (MacroLibrary)

# Make sure that we're having RPATH on our installed libs, else using kdevelop
# from prefixes like $HOME/kdevelop breaks
# Code taken from FindKDE4Internal.cmake for KDE 4.5
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${LIB_INSTALL_DIR}"
  _isSystemLibDir)
if("${_isSystemLibDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")
endif("${_isSystemLibDir}" STREQUAL "-1")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(KOMPARE_FOUND FALSE)
#macro_optional_find_package(Kompare)
macro_bool_to_01(KOMPARE_FOUND HAVE_KOMPARE)
#macro_log_feature(KOMPARE_FOUND "Kompare" "KPart to view file differences."
#                        "http://www.caffeinated.me.uk/kompare/" FALSE ""
#                        "Required for difference checking. From KDE SDK package, KDE 4.3.61 or higher needed.")


add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQURL_NO_CAST_FROM_QSTRING)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-kdevplatform.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-kdevplatform.h )

include_directories(${KDevPlatform_SOURCE_DIR} ${KDevPlatform_BINARY_DIR} ${KDE4_INCLUDES})
include_directories(${Grantlee_INCLUDE_DIRS})

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
#macro_optional_add_subdirectory(doc)
add_subdirectory(debugger)
add_subdirectory(documentation)
add_subdirectory(template)
add_subdirectory(tests)

add_subdirectory(plugins)

macro_write_basic_cmake_version_file( ${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake ${KDEVPLATFORM_VERSION_MAJOR} ${KDEVPLATFORM_VERSION_MINOR} ${KDEVPLATFORM_VERSION_PATCH} )
configure_file( "${KDevPlatform_SOURCE_DIR}/KDevPlatformConfig.cmake.in" "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake" @ONLY )
configure_file( "${KDevPlatform_SOURCE_DIR}/kdevplatformversion.h.cmake" "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h" @ONLY )

if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
    set(_kdevplatform_CONFIG_DEST "${LIB_INSTALL_DIR}/cmake/kdevplatform")
else(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
    set(_kdevplatform_CONFIG_DEST "${LIB_INSTALL_DIR}/kdevplatform")
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

install( FILES
        "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake"
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
        DESTINATION "${_kdevplatform_CONFIG_DEST}" )
install( EXPORT KDevPlatformTargets
         DESTINATION "${_kdevplatform_CONFIG_DEST}"
         NAMESPACE KDevPlatformImport__
         FILE KDevPlatformTargets.cmake )

# Put this last, so it can use _kdevplatform_CONFIG_DEST
add_subdirectory(cmake)

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)
