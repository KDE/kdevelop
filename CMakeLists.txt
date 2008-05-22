project(kdevplatform)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(KDE4 REQUIRED)
find_package(ZLIB REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (MacroLogFeature)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR}/threadweaver )

# create config.h
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

set(KDEVPLATFORM_LIB_VERSION 1.0.0)
set(KDEVPLATFORM_LIB_SOVERSION 1)

add_subdirectory(cmake)
add_subdirectory(sublime)
add_subdirectory(interfaces)
add_subdirectory(project)
add_subdirectory(editor)
add_subdirectory(language)
add_subdirectory(shell)
add_subdirectory(util)
add_subdirectory(outputview)
add_subdirectory(vcs)
add_subdirectory(plugins)
macro_optional_add_subdirectory(doc)

