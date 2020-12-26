add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustombuildsystem\")
declare_qt_logging_category(custom_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER CUSTOMBUILDSYSTEM
    CATEGORY_BASENAME "custombuildsystem"
)
## KDevelop Plugin
set(custom_SRCS
    custombuildsystemconfig.cpp
    custombuildsystemplugin.cpp
    custombuildjob.cpp
    configconstants.cpp
    configwidget.cpp
    custombuildsystemconfigwidget.cpp
    kcm_custombuildsystem.cpp
    ${custom_LOG_SRCS}
)
ki18n_wrap_ui(custom_SRCS configwidget.ui custombuildsystemconfigwidget.ui)
kconfig_add_kcfg_files(custom_SRCS kcfg_custombuildsystemconfig.kcfgc )

kdevplatform_add_plugin(kdevcustombuildsystem JSON kdevcustombuildsystem.json SOURCES ${custom_SRCS})
target_link_libraries(kdevcustombuildsystem
      KDev::Project
      KDev::Interfaces
      KDev::Util
      KDev::OutputView
      KDev::Tests
)

## Unittests
if(BUILD_TESTING)
    add_subdirectory( tests )
endif()
