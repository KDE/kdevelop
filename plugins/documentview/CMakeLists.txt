add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentview\")
project(documentview)
#add_subdirectory(settings)

########### next target ###############

set(kdevdocumentview_PART_SRCS
    kdevdocumentview.cpp
    kdevdocumentviewdelegate.cpp
    kdevdocumentviewplugin.cpp
    kdevdocumentmodel.cpp
    kdevdocumentselection.cpp
)

kdevplatform_add_plugin(kdevdocumentview JSON kdevdocumentview.json SOURCES ${kdevdocumentview_PART_SRCS})

target_link_libraries(kdevdocumentview
    KDev::Interfaces
    KF5::TextEditor
)

########### install files ###############

install( FILES kdevdocumentview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevdocumentview )

