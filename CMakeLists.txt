project(kdevelop)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_VERBOSE_MAKEFILE ON)

enable_testing()
include(CTest)

find_package(KDE4 REQUIRED)
find_package(Perl REQUIRED)
find_package(BerkeleyDB REQUIRED)
if (APPLE)
    find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})

include(MacroAdditionalCleanFiles)
include(MacroAddFileDependencies)
include (ConfigureChecks.cmake)

include_directories(${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# create config.h
include(ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )



add_subdirectory(lib) 
add_subdirectory(mimetypes) 
add_subdirectory(pics) 
add_subdirectory(src)

#add_subdirectory(parts) 
#add_subdirectory(buildtools) 
#add_subdirectory(doc) 
#add_subdirectory(languages) 



########### install files ###############

install_files(${XDG_APPS_DIR} FILES kdevelop.desktop kdevassistant.desktop)
install_files(${XDG_DIRECTORY_DIR} FILES kde-development-kdevelop.directory)
install_files(${MIME_INSTALL_DIR}/application FILES x-kdevelop.desktop)

install_files(/bin FILES kdevprj2kdevelop)


