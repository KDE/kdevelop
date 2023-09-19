add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentswitcher\")

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcheritem.cpp
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

declare_qt_logging_category(kdevdocumentswitcher_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_DOCUMENTSWITCHER
    CATEGORY_BASENAME "documentswitcher"
)

qt_add_resources(kdevdocumentswitcher_PART_SRCS kdevdocumentswitcher.qrc)
kdevplatform_add_plugin(kdevdocumentswitcher SOURCES ${kdevdocumentswitcher_PART_SRCS})

target_link_libraries(kdevdocumentswitcher KDev::Interfaces KDev::Sublime KDev::Interfaces KDev::Util KDev::Project )
