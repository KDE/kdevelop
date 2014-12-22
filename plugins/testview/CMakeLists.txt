add_definitions(-DTRANSLATION_DOMAIN=\"kdevtestview\")
########### next target ###############

set(kdevtestview_PLUGIN_SRCS
    testview.cpp
    testviewplugin.cpp
    testviewdebug.cpp
)

kdevplatform_add_plugin(kdevtestview JSON kdevtestview.json SOURCES ${kdevtestview_PLUGIN_SRCS})
target_link_libraries(kdevtestview KF5::ItemModels KDev::Interfaces KDev::Project KDev::Util KDev::Language)

########### install files ###############

install( FILES kdevtestview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevtestview )
