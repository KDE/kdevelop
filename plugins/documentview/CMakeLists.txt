add_definitions(-DTRANSLATION_DOMAIN=\"kdevdocumentview\")

set(kdevdocumentview_PART_SRCS
    kdevdocumentview.cpp
    kdevdocumentviewdelegate.cpp
    kdevdocumentviewplugin.cpp
    kdevdocumentmodel.cpp
    kdevdocumentselection.cpp
)

qt_add_resources(kdevdocumentview_PART_SRCS kdevdocumentview.qrc)
kdevplatform_add_plugin(kdevdocumentview SOURCES ${kdevdocumentview_PART_SRCS})

target_link_libraries(kdevdocumentview
    KDev::Interfaces
    KDev::Util
    KF6::TextEditor
)
