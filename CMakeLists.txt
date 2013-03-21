project(kdev-qml)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform 1.5.60 REQUIRED)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 5)
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
add_subdirectory(navigation)

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
)

install(FILES kdevqmljssupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kdevqmljslanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
