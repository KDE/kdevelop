project(kdevelop)

set(KDE4_BUILD_TESTS "ON")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set( KDevelop_VERSION_MAJOR 3 )
set( KDevelop_VERSION_MINOR 9 )
set( KDevelop_VERSION_PATCH 82 )

find_package(KDE4 REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)

find_package(KDevPlatform REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDevPlatform_INCLUDE_DIR} )

include(KDevelopMacros)

# create config.h
include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(languages)
add_subdirectory(plugins)
add_subdirectory(buildtools)
macro_optional_add_subdirectory(doc)

macro_display_feature_log()

