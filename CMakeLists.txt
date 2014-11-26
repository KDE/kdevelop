project(cppcheck)

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

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
    ${cppcheck_BINARY_DIR}
    ${cppcheck_SOURCE_DIR}
    ${cppcheck_SOURCE_DIR}/config
    ${cppcheck_SOURCE_DIR}/models
    ${cppcheck_SOURCE_DIR}/parsers
    ${cppcheck_SOURCE_DIR}/jobs
    ${cppcheck_SOURCE_DIR}/views
)


########### next target ###############

set(kdevcppcheck_PART_SRCS
    debug.cpp
    job.cpp
    marks.cpp
    imodel.cpp
    modelwrapper.cpp
    iparser.cpp
    plugin.cpp
    widget.cpp
    models/cppcheckmodel.cpp
    models/cppcheckitemsimpl.cpp
    models/cppcheck_file_model.cpp
    models/cppcheck_severity_model.cpp
    models/cppcheck_file_item.cpp
    models/cppcheck_severity_item.cpp
    parsers/cppcheckparser.cpp
    views/cppcheckview.cpp
)

set(kdevcppcheck_CONFIG_SRCS

)

set(kdevcppcheck_PART_UI
)

ki18n_wrap_ui(kdevcppcheck_PART_SRCS  ${kdevcppcheck_PART_UI})

add_library(kdevcppcheck MODULE ${kdevcppcheck_PART_SRCS} ${kdevcppcheck_CONFIG_SRCS})

target_link_libraries(kdevcppcheck
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
    KF5::ConfigCore
    KF5::IconThemes
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::TextEditor
    Qt5::Network)

install(TARGETS kdevcppcheck DESTINATION ${PLUGIN_INSTALL_DIR} )
########### install files ###############

configure_file(kdevcppcheck.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcppcheck.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcppcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevcppcheck.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppcheck)

#preferences-plugin
set(cppcheckpreferences_PART_SRCS 
    debug.cpp
    config/genericconfigpage.cpp
)
ki18n_wrap_ui(cppcheckpreferences_PART_SRCS config/ui/genericconfig.ui)

add_library(kcm_kdev_cppcheck MODULE ${cppcheckpreferences_PART_SRCS})

target_link_libraries(kcm_kdev_cppcheck
    KDev::Interfaces
    KF5::KIOCore
)

install(TARGETS kcm_kdev_cppcheck  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kcm_kdev_cppcheck.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

