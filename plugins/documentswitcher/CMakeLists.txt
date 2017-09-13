add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentswitcher\")

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

ecm_qt_declare_logging_category(kdevdocumentswitcher_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_DOCUMENTSWITCHER
    CATEGORY_NAME "kdevplatform.plugins.documentswitcher"
)

qt5_add_resources(kdevdocumentswitcher_PART_SRCS kdevdocumentswitcher.qrc)
kdevplatform_add_plugin(kdevdocumentswitcher JSON kdevdocumentswitcher.json SOURCES ${kdevdocumentswitcher_PART_SRCS})

target_link_libraries(kdevdocumentswitcher KDev::Interfaces KDev::Sublime KDev::Interfaces KDev::Util KDev::Project )
