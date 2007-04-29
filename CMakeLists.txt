project(kdevelop)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)


find_package(ZLIB REQUIRED)
find_package(KDE4 REQUIRED)
#find_package(BerkeleyDB)
#find_package(TDB)
include (KDE4Defaults)
include (MacroLibrary)

find_package(Commoncpp)
find_package(Boost)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR}/threadweaver )

include(KDevelopMacros)

# create config.h
include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory(cmake)
add_subdirectory(lib)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(languages)
add_subdirectory(plugins)
add_subdirectory(buildtools)
add_subdirectory(doc)

