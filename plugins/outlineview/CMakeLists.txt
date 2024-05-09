add_definitions(-DTRANSLATION_DOMAIN=\"kdevoutlineview\")

set(kdevoutlineview_SRCS
    outlineviewplugin.cpp
    outlinenode.cpp
    outlinemodel.cpp
    outlinewidget.cpp
)

declare_qt_logging_category(kdevoutlineview_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_OUTLINE
    CATEGORY_BASENAME "outline"
)
kdevplatform_add_plugin(KDevOutlineView SOURCES ${kdevoutlineview_SRCS})
target_link_libraries(KDevOutlineView
    KDev::Interfaces
    KDev::Language
    KF6::CoreAddons
    KF6::I18n
    KF6::ItemModels
    KF6::TextEditor
)
