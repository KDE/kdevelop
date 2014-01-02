project(kdev-clang)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 5)
set(VERSION_PATCH 60)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

cmake_minimum_required(VERSION 2.8.9)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

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

add_subdirectory(duchain)
add_subdirectory(tests)

kde4_add_plugin(kdevclanglanguagesupport
    clangparsejob.cpp
    clanglanguagesupport.cpp
    clanghighlighting.cpp
)
target_link_libraries(kdevclanglanguagesupport
    kdevclangduchain

    ${KDE4_THREADWEAVER_LIBS}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

configure_file(kdevclangsupport.desktop.cmake ${CMAKE_BINARY_DIR}/kdevclangsupport.desktop)
install( FILES ${CMAKE_BINARY_DIR}/kdevclangsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevclanglanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
