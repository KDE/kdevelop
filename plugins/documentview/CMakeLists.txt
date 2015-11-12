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

qt5_add_resources(kdevdocumentview_PART_SRCS kdevdocumentview.qrc)
kdevplatform_add_plugin(kdevdocumentview JSON kdevdocumentview.json SOURCES ${kdevdocumentview_PART_SRCS})

target_link_libraries(kdevdocumentview
    KDev::Interfaces
    KF5::TextEditor
)
