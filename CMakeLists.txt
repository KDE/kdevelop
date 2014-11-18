project(QMakeManager)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set(KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

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
find_package(KDevelop-PG-Qt REQUIRED)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)
enable_testing()

include_directories(${KDEVELOP_INCLUDE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(qmakebuilder)
add_subdirectory(app_templates)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9024 )

include_directories(
    ${qmakebuilder_SOURCE_DIR}
    ${QMakeManager_SOURCE_DIR}
    ${QMakeManager_BINARY_DIR}
)

add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(tests)

include_directories(${QMakeManager_SOURCE_DIR}/parser)

########### next target ###############

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectfile.cpp
    qmakeincludefile.cpp
    qmakemkspecs.cpp
    qmakefile.cpp
    qmakefilevisitor.cpp
    qmakecache.cpp
    variablereferenceparser.cpp
    qmakejob.cpp
    qmakebuilddirchooser.cpp
    qmakebuilddirchooserdialog.cpp
    qmakeconfig.cpp
)

set(kdevqmakemanager_PART_UI
    qmakebuilddirchooser.ui
)

ki18n_wrap_ui( kdevqmakemanager_PART_SRCS ${kdevqmakemanager_PART_UI} )

add_library(kdevqmakemanager MODULE ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager
    KF5::KDELibs4Support
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Util
    KDev::OutputView
    KDev::Serialization
    kdev4qmakeparser )

########### install files ###############

configure_file(kdevqmakemanager.desktop.cmake ${CMAKE_BINARY_DIR}/kdevqmakemanager.desktop)
install( FILES ${CMAKE_BINARY_DIR}/kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

########### cdash / unit tests ###############

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)
