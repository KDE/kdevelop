# kdevplatform soversion
# E.g. for KDevelop 5.2.0 => SOVERSION 52 (we only promise ABI compatibility between patch version updates)
set(KDEVPLATFORM_SOVERSION ${KDEVELOP_SOVERSION})

# Increase this to reset incompatible item-repositories.
# Changing KDEVELOP_VERSION automatically resets the itemrepository as well.
set(KDEV_ITEMREPOSITORY_INCREMENT 0)

set(KDevPlatform_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KDevPlatform_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KDevPlatform")

include(KDevPlatformMacros)
include(KDevPlatformMacrosInternal)

find_package(Grantlee5 CONFIG)
set_package_properties(Grantlee5 PROPERTIES
    PURPOSE "Grantlee templating library, needed for file templates"
    URL "http://www.grantlee.org/"
    TYPE REQUIRED)

set(Boost_ADDITIONAL_VERSIONS 1.39.0 1.39)
find_package(Boost 1.35.0)
set_package_properties(Boost PROPERTIES
    PURPOSE "Boost libraries for enabling the classbrowser"
    URL "http://www.boost.org"
    TYPE REQUIRED)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-kdevplatform.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-kdevplatform.h )

include_directories(${KDevPlatform_SOURCE_DIR} ${KDevPlatform_BINARY_DIR})

if(BUILD_TESTING)
  set(KDEV_FIND_DEP_QT5TEST "find_dependency(Qt5Test \"${QT_MIN_VERSION}\")")
endif()
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDevPlatformConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDevPlatformConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KDEVPLATFORM
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdevplatform_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
                  SOVERSION ${KDEVPLATFORM_SOVERSION}
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug"
  OR CMAKE_BUILD_TYPE_TOLOWER STREQUAL "")
  set(COMPILER_OPTIMIZATIONS_DISABLED TRUE)
else()
  set(COMPILER_OPTIMIZATIONS_DISABLED FALSE)
endif()

add_subdirectory(sublime)
add_subdirectory(interfaces)
add_subdirectory(project)
add_subdirectory(language)
add_subdirectory(shell)
add_subdirectory(util)
add_subdirectory(outputview)
add_subdirectory(vcs)
add_subdirectory(pics)
add_subdirectory(debugger)
add_subdirectory(documentation)
add_subdirectory(serialization)
add_subdirectory(template)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()


install( FILES
        "${KDevPlatform_BINARY_DIR}/kdevplatform_version.h"
        "${KDevPlatform_BINARY_DIR}/config-kdevplatform.h"
        DESTINATION "${KDE_INSTALL_INCLUDEDIR}/kdevplatform" )
install( FILES
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfig.cmake"
        "${KDevPlatform_BINARY_DIR}/KDevPlatformConfigVersion.cmake"
        cmake/modules/KDevPlatformMacros.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}" )
install( EXPORT KDevPlatformTargets
         DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
         NAMESPACE KDev::
         FILE KDevPlatformTargets.cmake )

# kdebugsettings file
install( FILES kdevplatform.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
