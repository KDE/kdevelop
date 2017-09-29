add_definitions(-DTRANSLATION_DOMAIN=\"kdevtestview\")
########### next target ###############

set(kdevtestview_PLUGIN_SRCS
    testview.cpp
    testviewplugin.cpp
)

ecm_qt_declare_logging_category(kdevtestview_PLUGIN_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_TESTVIEW
    CATEGORY_NAME "kdevelop.plugins.testview"
)
qt5_add_resources(kdevtestview_PLUGIN_SRCS kdevtestview.qrc)
kdevplatform_add_plugin(kdevtestview JSON kdevtestview.json SOURCES ${kdevtestview_PLUGIN_SRCS})
target_link_libraries(kdevtestview KF5::ItemModels KDev::Interfaces KDev::Util KDev::Language)
