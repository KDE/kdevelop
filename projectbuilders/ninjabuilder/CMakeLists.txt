add_definitions(-DTRANSLATION_DOMAIN=\"kdevninja\")
set(kdevninja_SRCS ninjajob.cpp ninjabuilder.cpp ninjabuilderpreferences.cpp)
ki18n_wrap_ui(kdevninja_SRCS ninjaconfig.ui)
kconfig_add_kcfg_files(kdevninja_SRCS ninjabuilderconfig.kcfgc)

kdevplatform_add_plugin(kdevninja JSON kdevninja.json SOURCES ${kdevninja_SRCS})

target_link_libraries(kdevninja
    KDev::Interfaces
    KDev::Project
    KDev::OutputView
    KDev::Shell
    KDev::Util
)
