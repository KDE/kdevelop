add_definitions(-DTRANSLATION_DOMAIN=\"kdevtestview\")
########### next target ###############

set(kdevtestview_PLUGIN_SRCS
    testview.cpp
    testviewplugin.cpp
)

declare_qt_logging_category(kdevtestview_PLUGIN_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_TESTVIEW
    CATEGORY_BASENAME "testview"
)
qt_add_resources(kdevtestview_PLUGIN_SRCS kdevtestview.qrc)
kdevplatform_add_plugin(kdevtestview SOURCES ${kdevtestview_PLUGIN_SRCS})
target_link_libraries(kdevtestview KF6::ItemModels KDev::Interfaces KDev::Util KDev::Language)
