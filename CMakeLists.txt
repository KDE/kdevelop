# KDevelop version
set( KDEVELOP_VERSION_MAJOR 4 )
set( KDEVELOP_VERSION_MINOR 90 )
set( KDEVELOP_VERSION_PATCH 60 )
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "1.${KDEVELOP_VERSION_MINOR}.${KDEVELOP_VERSION_PATCH}" )

################################################################################

cmake_minimum_required(VERSION 2.8.12)

project(KDevelop)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Concurrent Declarative Test)
find_package(KF5 CONFIG REQUIRED COMPONENTS
  Archive
  Config
  DocTools
  GuiAddons
  I18n
  Init
  ItemModels
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  Parts
  PrintUtils
  Sonnet
  TextEditor
  ThreadWeaver
  WindowSystem
  XmlGui
  TextEditor
  NotifyConfig
  ThreadWeaver
  ItemModels
  Service
  Emoticons
  KCMUtils
  ItemViews
  Plasma

  # TODO KF5: Port away from this
  KDE4Support
)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(${KDevelop_SOURCE_DIR} ${KDevelop_BINARY_DIR})

# create config.h
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(app)
add_subdirectory(formatters)
add_subdirectory(languages)
add_subdirectory(projectbuilders)
add_subdirectory(projectmanagers)
add_subdirectory(debuggers)
add_subdirectory(app_templates)
add_subdirectory(documentation)
add_subdirectory(kdeintegration)
add_subdirectory(utils)
add_subdirectory(file_templates)
add_subdirectory(providers)

add_subdirectory(doc)

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
