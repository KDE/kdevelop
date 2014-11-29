project(cmakebuilder)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcmakebuilder\")
include_directories(
    ${cmakebuilder_SOURCE_DIR}
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    )


########### next target ###############

set(cmakebuilder_SRCS
    cmakebuilder.cpp
    cmakejob.cpp
    prunejob.cpp
    cmakebuilderpreferences.cpp
)

ki18n_wrap_ui(cmakebuilder_SRCS cmakebuildersettings.ui)
kconfig_add_kcfg_files( cmakebuilder_SRCS cmakebuilderconfig.kcfgc )
add_library(kdevcmakebuilder MODULE ${cmakebuilder_SRCS})
target_link_libraries(
    kdevcmakebuilder
    kdevcmakecommon
        KF5::KIOWidgets

        KDev::Interfaces
        KDev::OutputView
        KDev::Util
        KDev::Shell
        KDev::Project
)

install(TARGETS kdevcmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevcmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
