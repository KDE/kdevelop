project(KDevelop)

set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set( KDEVELOP_VERSION_MAJOR 4 )
set( KDEVELOP_VERSION_MINOR 0 )
set( KDEVELOP_VERSION_PATCH 60 )

set(KDE_MIN_VERSION "4.3.0")
find_package(KDE4 4.3.0 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)

find_package(KDevPlatform 1.0.0 REQUIRED)
include_directories(${KDEVPLATFORM_INCLUDE_DIR})

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${KDevelop_SOURCE_DIR} ${KDevelop_BINARY_DIR} ${KDE4_INCLUDES} )

# create config.h
include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(app)
add_subdirectory(formatters)
add_subdirectory(languages)
add_subdirectory(projectbuilders)
add_subdirectory(projectmanagers)
add_subdirectory(debuggers)
add_subdirectory(app_templates)
add_subdirectory(documentation)
#macro_optional_add_subdirectory(doc)

macro_display_feature_log()

