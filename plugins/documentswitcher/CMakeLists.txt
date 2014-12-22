add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentswitcher\")
project(documentswitcher)

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

kdevplatform_add_plugin(kdevdocumentswitcher JSON kdevdocumentswitcher.json SOURCES ${kdevdocumentswitcher_PART_SRCS})

target_link_libraries(kdevdocumentswitcher KDev::Interfaces KDev::Sublime KDev::Interfaces )

########### install files ###############

install( FILES kdevdocumentswitcher.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevdocumentswitcher )


