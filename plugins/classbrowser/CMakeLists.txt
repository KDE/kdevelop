add_definitions(-DTRANSLATION_DOMAIN=\"kdevclassbrowser\")
include_directories(
    ${Boost_INCLUDE_DIRS}
)

set(kdevclassbrowser_PART_SRCS
    classbrowserplugin.cpp
    classwidget.cpp
    classtree.cpp
)
declare_qt_logging_category(kdevclassbrowser_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_CLASSBROWSER
    CATEGORY_BASENAME "classbrowser"
)
qt_add_resources(kdevclassbrowser_PART_SRCS kdevclassbrowser.qrc)
kdevplatform_add_plugin(kdevclassbrowser SOURCES ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser KF6::TextEditor KDev::Util KDev::Language KDev::Interfaces)
