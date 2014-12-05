# KDevelop version
set( KDEVELOP_VERSION_MAJOR 4 )
set( KDEVELOP_VERSION_MINOR 90 )
set( KDEVELOP_VERSION_PATCH 90 )
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

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Concurrent Quick WebKitWidgets Script Test)
set(KF5_DEP_VERSION "5.3.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
  Config
  I18n
  ItemModels
  ItemViews
  JobWidgets
  KCMUtils
  KIO
  NewStuff
  NotifyConfig
  Parts
  Service
  TextEditor
  ThreadWeaver
  XmlGui
  WindowSystem

  # TODO KF5: Port away from this
  KDELibs4Support
)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0) # comes from KDELibs4Support
add_definitions(-DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x050000)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Turn off missing-field-initializers warning to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()

include_directories(${KDevelop_SOURCE_DIR} ${KDevelop_BINARY_DIR}  )

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

install(FILES kdevelop.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
