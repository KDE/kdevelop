# KDevelop version
set( KDEVELOP_VERSION_MAJOR 4 )
set( KDEVELOP_VERSION_MINOR 90 )
set( KDEVELOP_VERSION_PATCH 92 )
set( KDEVELOP_VERSION "${KDEVELOP_VERSION_MAJOR}.${KDEVELOP_VERSION_MINOR}.${KDEVELOP_VERSION_PATCH}" )

################################################################################

cmake_minimum_required(VERSION 2.8.12)

project(KDevelop)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)

include(CTest)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(QT_MIN_VERSION "5.4.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Concurrent Quick QuickWidgets WebKitWidgets Script Test)
set(KF5_DEP_VERSION "5.15.0") # we need KCrash::initialize
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
  Config
  Declarative
  DocTools
  IconThemes
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
  Crash
)
find_package(KF5SysGuard)
set_package_properties(KF5SysGuard PROPERTIES
    PURPOSE "Framework for process listing. Required for the 'Attach to Process' feature"
)
find_package(KDevelop-PG-Qt 1.90.90)
set_package_properties(KDevelop-PG-Qt PROPERTIES
    PURPOSE "KDevelop parser generator library. Required for the QMake Builder/Manager plugin."
)

find_package(KDevPlatform ${KDEVELOP_VERSION} REQUIRED)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)

# Turn off missing-field-initializers warning to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
check_cxx_compiler_flag(-Wno-missing-field-initializers HAVE_MFI_FLAG)
check_cxx_compiler_flag(-Werror=undefined-bool-conversion HAVE_UBC_FLAG)
check_cxx_compiler_flag(-Werror=tautological-undefined-compare HAVE_TUC_FLAG)
if (HAVE_MFI_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()
if (HAVE_UBC_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=undefined-bool-conversion")
endif()
if (HAVE_TUC_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=tautological-undefined-compare")
endif()

include_directories(${KDevelop_SOURCE_DIR} ${KDevelop_BINARY_DIR}  )

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug"
  OR CMAKE_BUILD_TYPE_TOLOWER STREQUAL "")
  set(COMPILER_OPTIMIZATIONS_DISABLED TRUE)
else()
  set(COMPILER_OPTIMIZATIONS_DISABLED FALSE)
endif()

# create config.h
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

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
add_subdirectory(shortcuts)
add_subdirectory(doc)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDevelop")
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevelopConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfig.cmake"
                                  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
ecm_setup_version(${KDEVELOP_VERSION_MAJOR}.${KDEVELOP_VERSION_MINOR}.${KDEVELOP_VERSION_PATCH}
                  VARIABLE_PREFIX KDEVELOP
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdevelop_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfigVersion.cmake"
                  SOVERSION ${KDEVELOP_LIB_SOVERSION}
)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kdevelop_version.h"
        DESTINATION "${KDE_INSTALL_INCLUDEDIR}/kdevelop")
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KDevelopConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}" )
install(EXPORT KDevelopTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" NAMESPACE KDev:: FILE KDevelopTargets.cmake)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

install(FILES org.kde.kdevelop.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
