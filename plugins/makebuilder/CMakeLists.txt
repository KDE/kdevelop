add_definitions(-DTRANSLATION_DOMAIN=\"kdevmakebuilder\")

########### next target ###############

set(kdevmakebuilder_SRCS
    makebuilder.cpp
    makejob.cpp
    makebuilderpreferences.cpp
)
declare_qt_logging_category(kdevmakebuilder_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_MAKEBUILDER
    CATEGORY_BASENAME "makebuilder"
)
ki18n_wrap_ui(kdevmakebuilder_SRCS makeconfig.ui)
kconfig_add_kcfg_files( kdevmakebuilder_SRCS makebuilderconfig.kcfgc )

qt_add_resources(kdevmakebuilder_SRCS kdevmakebuilder.qrc)
kdevplatform_add_plugin(KDevMakeBuilder SOURCES ${kdevmakebuilder_SRCS})
target_link_libraries(KDevMakeBuilder
        KF6::TextEditor
        KF6::KIOWidgets
        KDev::Interfaces
        KDev::Project
        KDev::OutputView
        KDev::Shell
        KDev::Util
        KDev::Shell
)

install(FILES imakebuilder.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevelop/makebuilder COMPONENT Devel)

add_library(KDevIMakeBuilder INTERFACE)
add_library(KDev::IMakeBuilder ALIAS KDevIMakeBuilder)
target_include_directories(KDevIMakeBuilder INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevelop>"
)
set_target_properties(KDevIMakeBuilder PROPERTIES
    EXPORT_NAME IMakeBuilder
)
install(TARGETS KDevIMakeBuilder EXPORT KDevelopTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
