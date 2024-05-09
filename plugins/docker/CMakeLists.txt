kdevplatform_add_file_templates(dockerfile-template)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocker\")

declare_qt_logging_category(dockerplugin_SRCS
    TYPE PLUGIN
    HEADER debug_docker.h
    IDENTIFIER DOCKER
    CATEGORY_BASENAME "docker"
)
qt_add_resources(dockerplugin_SRCS kdevdockerplugin.qrc)
ki18n_wrap_ui(dockerplugin_SRCS dockerpreferences.ui)
kconfig_add_kcfg_files(dockerplugin_SRCS dockerpreferencessettings.kcfgc)

kdevplatform_add_plugin(kdevdocker SOURCES dockerplugin.cpp dockerruntime.cpp dockerpreferences.cpp ${dockerplugin_SRCS})
target_link_libraries(kdevdocker
    KF6::CoreAddons
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)
