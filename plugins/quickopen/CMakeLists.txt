add_definitions(-DTRANSLATION_DOMAIN=\"kdevquickopen\")

declare_qt_logging_category(kdevquickopen_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_QUICKOPEN
    CATEGORY_BASENAME "quickopen"
)

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
    ${kdevquickopen_LOG_SRCS}
)
ki18n_wrap_ui(kdevquickopen_PART_SRCS
  quickopenwidget.ui
)
qt5_add_resources(kdevquickopen_PART_SRCS kdevquickopen.qrc)
kdevplatform_add_plugin(kdevquickopen JSON kdevquickopen.json SOURCES ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen
    KDev::Language
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KF5::GuiAddons
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
