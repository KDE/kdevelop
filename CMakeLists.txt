project(kdev-clang)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

cmake_minimum_required(VERSION 2.8.12)
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS KDELibs4Support ThreadWeaver TextEditor I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
find_package(KDevelop REQUIRED)

find_package(Clang 3.4)
set_package_properties(Clang PROPERTIES
    TYPE REQUIRED
    PURPOSE "LibClang is a high level C interface to clang"
    URL "http://clang.llvm.org/")

add_definitions(${LLVM_CFLAGS})
include_directories(
    ${CLANG_INCLUDE_DIR}
)

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

enable_testing()

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

include_directories(
    ${KDEVELOP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(duchain)
add_subdirectory(codecompletion)
add_subdirectory(codegen)
add_subdirectory(util)
add_subdirectory(tests)

set(kdevclangsupport_SRCS
    clangparsejob.cpp
    clangsupport.cpp
    documentfinderhelpers.cpp
)

kdevplatform_add_plugin(kdevclangsupport JSON kdevclangsupport.json SOURCES ${kdevclangsupport_SRCS})

target_link_libraries(kdevclangsupport
    kdevclangduchain
    kdevclangcodecompletion
    kdevclangcodegen
    kdevclangutil

    KF5::ThreadWeaver
    KF5::TextEditor
    KDev::Util
    KDev::Project
)

install(FILES kdevclangsupport.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevclangsupport)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
