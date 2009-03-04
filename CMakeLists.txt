project(kdevelop)

set(KDE4_BUILD_TESTS "ON")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set( KDEVELOP_VERSION_MAJOR 3 )
set( KDEVELOP_VERSION_MINOR 9 )
set( KDEVELOP_VERSION_PATCH 91 )

set(KDE_MIN_VERSION "4.2.1")
find_package(KDE4 4.2.1 REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)

find_package(KDevPlatform 0.9.91 REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} )

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
add_subdirectory(tools)
add_subdirectory(xtest)
add_subdirectory(app_templates)
add_subdirectory(documentation)
macro_optional_add_subdirectory(doc)

macro_display_feature_log()

