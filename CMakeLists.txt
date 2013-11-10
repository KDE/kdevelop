project(kdev-qml)

cmake_minimum_required(VERSION 2.8)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform 1.6.60 REQUIRED)
find_package(KDeclarative REQUIRED)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 6)
set(VERSION_PATCH 60)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

enable_testing()
set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_definitions(-DQT_CREATOR) # :D
add_definitions(-DQML_BUILD_STATIC_LIB=0 -DQMLJS_BUILD_DIR)

add_subdirectory(libs)
add_subdirectory(duchain)
add_subdirectory(tests)
add_subdirectory(codecompletion)

kde4_add_plugin(kdevqmljslanguagesupport
    qmljsparsejob.cpp
    qmljshighlighting.cpp
    kdevqmljsplugin.cpp
    navigation/colorchooser.cpp
    navigation/propertypreviewwidget.cpp
)
target_link_libraries(kdevqmljslanguagesupport
    kdevqmljsduchain
    kdevqmljscompletion
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTDECLARATIVE_LIBRARIES}
    ${KDECLARATIVE_LIBRARIES}
)

install(TARGETS kdevqmljslanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
install(DIRECTORY navigation/propertywidgets DESTINATION ${DATA_INSTALL_DIR})

configure_file(kdevqmljssupport.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqmljssupport.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqmljssupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
