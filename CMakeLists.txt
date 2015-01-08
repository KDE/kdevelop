project(kdev-qml)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

cmake_minimum_required(VERSION 2.8.12)
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED Core Declarative Widgets Test Qml QuickWidgets)
find_package(KF5 REQUIRED COMPONENTS ItemModels ThreadWeaver TextEditor Declarative I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
find_package(KDevelop REQUIRED)

enable_testing()

include_directories(
    ${KDEVELOP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_definitions(-DQT_CREATOR) # :D
add_definitions(-DQML_BUILD_STATIC_LIB=0 -DQMLJS_BUILD_DIR)

add_subdirectory(libs)
add_subdirectory(duchain)
add_subdirectory(nodejsmodules)
add_subdirectory(tests)
add_subdirectory(codecompletion)

add_library(kdevqmljslanguagesupport MODULE
    qmljsparsejob.cpp
    qmljshighlighting.cpp
    kdevqmljsplugin.cpp
    debug.cpp
    navigation/propertypreviewwidget.cpp
)
target_link_libraries(kdevqmljslanguagesupport
    kdevqmljsduchain
    kdevqmljscompletion
    Qt5::Widgets
    Qt5::Declarative
    Qt5::QuickWidgets
    KF5::Declarative
)

install(TARGETS kdevqmljslanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(DIRECTORY navigation/propertywidgets DESTINATION ${DATA_INSTALL_DIR})

configure_file(kdevqmljssupport.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqmljssupport.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqmljssupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
