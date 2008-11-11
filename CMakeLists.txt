cmake_minimum_required(VERSION 2.6)

project(KDevPlatform)

set(KDE4_BUILD_TESTS "ON")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(KDevPlatform_VERSION_MAJOR 0)
set(KDevPlatform_VERSION_MINOR 9)
set(KDevPlatform_VERSION_PATCH 82)

set(KDE_MIN_VERSION "4.1.70")
find_package(KDE4 4.1.70 REQUIRED)
find_package(ZLIB REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (MacroLogFeature)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} )

set(KDEVPLATFORM_LIB_VERSION 1.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 1)

add_subdirectory(cmake)
add_subdirectory(sublime)
add_subdirectory(interfaces)
add_subdirectory(project)
add_subdirectory(language)
add_subdirectory(shell)
add_subdirectory(util)
add_subdirectory(outputview)
add_subdirectory(vcs)
add_subdirectory(plugins)
add_subdirectory(veritas)
if(NOT WIN32)
    macro_optional_add_subdirectory(kross)
endif(NOT WIN32)
macro_optional_add_subdirectory(doc)

configure_file( "${KDevPlatform_SOURCE_DIR}/KDevPlatformConfigVersion.cmake.in" "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake" @ONLY )
configure_file( "${KDevPlatform_SOURCE_DIR}/KDevPlatformConfig.cmake.in" "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake" @ONLY )
configure_file( "${KDevPlatform_SOURCE_DIR}/kdevplatformversion.h.cmake" "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h" @ONLY )

install( FILES 
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake" 
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake" 
        DESTINATION "${LIB_INSTALL_DIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/kdevplatformversion.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}/kdevplatform" )
