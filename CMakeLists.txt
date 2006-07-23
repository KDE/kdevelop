project(kdevelop)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

find_package(BerkeleyDB REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include(KDevelopMacros)

# create config.h
include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


add_subdirectory(lib)
add_subdirectory(mimetypes)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(languages)
add_subdirectory(plugins)
add_subdirectory(buildtools)
add_subdirectory(menus)
#add_subdirectory(doc)

