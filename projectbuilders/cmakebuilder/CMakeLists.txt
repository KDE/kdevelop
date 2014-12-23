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
kdevplatform_add_plugin(kdevcmakebuilder JSON kdevcmakebuilder.json SOURCES ${cmakebuilder_SRCS})
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
