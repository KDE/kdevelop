project(KDevelop)

set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set( KDEVELOP_VERSION_MAJOR 3 )
set( KDEVELOP_VERSION_MINOR 10 )
set( KDEVELOP_VERSION_PATCH 1 )

set(KDE_MIN_VERSION "4.3.0")
find_package(KDE4 4.3.0 REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)

if( extragear-sdk_SOURCE_DIR )
    if ( ${extragear-sdk_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}  )
        set( KDEVPLATFORM_FOUND TRUE )
        include_directories( ${CMAKE_SOURCE_DIR}/kdevplatform ${CMAKE_SOURCE_DIR}/kdevplatform/plugins )
        set( KDEVPLATFORM_INTERFACES_LIBRARIES kdevplatforminterfaces )
        set( KDEVPLATFORM_SHELL_LIBRARIES kdevplatformshell )
        set( KDEVPLATFORM_TESTS_LIBRARIES kdevplatformtests )
        set( KDEVPLATFORM_UTIL_LIBRARIES kdevplatformutil )
        set( KDEVPLATFORM_PROJECT_LIBRARIES kdevplatformproject )
        set( KDEVPLATFORM_OUTPUTVIEW_LIBRARIES kdevplatformoutputview )
        set( KDEVPLATFORM_LANGUAGE_LIBRARIES kdevplatformlanguage )
        set( KDEVPLATFORM_VCS_LIBRARIES kdevplatformvcs )
        set( KDEVPLATFORM_DEBUGGER_LIBRARIES kdevplatformdebugger )
        set( KDEVPLATFORM_SUBLIME_LIBRARIES sublime )
        include( ${CMAKE_SOURCE_DIR}/kdevplatform/cmake/modules/KDevPlatformMacros.cmake )
    endif ( ${extragear-sdk_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}  )
endif( extragear-sdk_SOURCE_DIR )

if( NOT KDEVPLATFORM_FOUND )

    find_package(KDevPlatform 0.10.1 REQUIRED)
    include_directories(${KDEVPLATFORM_INCLUDE_DIR})

endif( NOT KDEVPLATFORM_FOUND )

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

