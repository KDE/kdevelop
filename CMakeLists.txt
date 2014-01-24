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

find_package(Clang 3.3 REQUIRED)

# TODO: Can we make this implicit via set_target_properties magic?
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})
include_directories(
    ${LLVM_INCLUDE_DIRS}
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # in Qt 4, clang support is lacking...
    add_definitions("-DQ_COMPILER_INITIALIZER_LISTS=1")
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

enable_testing()

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(duchain)
add_subdirectory(codecompletion)
add_subdirectory(tests)

kde4_add_plugin(kdevclanglanguagesupport
    clangparsejob.cpp
    clanglanguagesupport.cpp
)
target_link_libraries(kdevclanglanguagesupport
    kdevclangduchain
    kdevclangcodecompletion

    ${KDE4_THREADWEAVER_LIBS}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

configure_file(kdevclangsupport.desktop.cmake ${CMAKE_BINARY_DIR}/kdevclangsupport.desktop)
install( FILES ${CMAKE_BINARY_DIR}/kdevclangsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevclanglanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
