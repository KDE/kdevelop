add_definitions(-DTRANSLATION_DOMAIN=\"kdevquickopen\")
if(BUILD_TESTING)
    add_subdirectory( tests )
endif()

set(kdevquickopen_PART_SRCS
    quickopenplugin.cpp
    quickopenmodel.cpp
    quickopenwidget.cpp
    projectfilequickopen.cpp
    duchainitemquickopen.cpp
    declarationlistquickopen.cpp
    projectitemquickopen.cpp
    documentationquickopenprovider.cpp
    actionsquickopenprovider.cpp
    expandingtree/expandingdelegate.cpp
    expandingtree/expandingtree.cpp
    expandingtree/expandingwidgetmodel.cpp
)
declare_qt_logging_category(kdevquickopen_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_QUICKOPEN
    CATEGORY_BASENAME "quickopen"
)
ki18n_wrap_ui(kdevquickopen_PART_SRCS
  quickopenwidget.ui
)
qt_add_resources(kdevquickopen_PART_SRCS kdevquickopen.qrc)
kdevplatform_add_plugin(kdevquickopen SOURCES ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen
    KDev::Language
    KDev::Interfaces
    KDev::Project
    KDev::Util
    gfx::timsort
    KF6::GuiAddons
    KF6::TextWidgets
)
