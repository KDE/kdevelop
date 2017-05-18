add_subdirectory(tests)

add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocker\")

ecm_qt_declare_logging_category(dockerplugin_SRCS
    HEADER debug_docker.h
    IDENTIFIER DOCKER
    CATEGORY_NAME kdevplatform.plugins.docker
)
qt5_add_resources(dockerplugin_SRCS kdevdockerplugin.qrc)
ki18n_wrap_ui(dockerplugin_SRCS dockerpreferences.ui)
kconfig_add_kcfg_files(dockerplugin_SRCS dockerpreferencessettings.kcfgc)

kdevplatform_add_plugin(kdevdocker JSON kdevdocker.json SOURCES dockerplugin.cpp dockerruntime.cpp dockerpreferences.cpp ${dockerplugin_SRCS})
target_link_libraries(kdevdocker
    KF5::CoreAddons
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)
