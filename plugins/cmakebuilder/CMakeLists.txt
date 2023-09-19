add_definitions(-DTRANSLATION_DOMAIN=\"kdevcmakebuilder\")

########### next target ###############

set(cmakebuilder_SRCS
    cmakebuilder.cpp
    cmakejob.cpp
    prunejob.cpp
    cmakebuilderpreferences.cpp
)

declare_qt_logging_category(cmakebuilder_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_CMAKEBUILDER
    CATEGORY_BASENAME "cmakebuilder"
)
ki18n_wrap_ui(cmakebuilder_SRCS cmakebuilderpreferences.ui)
kdevplatform_add_plugin(KDevCMakeBuilder SOURCES ${cmakebuilder_SRCS})
target_link_libraries(KDevCMakeBuilder
    KDevCMakeCommon
    KDev::Interfaces
    KDev::OutputView
    KDev::Util
    KDev::Shell
    KDev::Project
    KDev::IMakeBuilder
    KF6::KIOWidgets
)
