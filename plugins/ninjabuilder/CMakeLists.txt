add_definitions(-DTRANSLATION_DOMAIN=\"kdevninja\")
set(kdevninja_SRCS ninjajob.cpp ninjabuilder.cpp ninjabuilderpreferences.cpp)
declare_qt_logging_category(kdevninja_SRCS
    TYPE PLUGIN
    IDENTIFIER NINJABUILDER
    CATEGORY_BASENAME "ninjabuilder"
)
ki18n_wrap_ui(kdevninja_SRCS ninjaconfig.ui)
kconfig_add_kcfg_files(kdevninja_SRCS ninjabuilderconfig.kcfgc)

kdevplatform_add_plugin(KDevNinjaBuilder SOURCES ${kdevninja_SRCS})

target_link_libraries(KDevNinjaBuilder
    KDev::Interfaces
    KDev::Project
    KDev::OutputView
    KDev::Shell
    KDev::Util
)
