add_definitions(-DTRANSLATION_DOMAIN=\"kdevstandardoutputview\")
add_subdirectory(tests)

########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
    standardoutputviewmetadata.cpp
)

kdevplatform_add_plugin(kdevstandardoutputview JSON kdevstandardoutputview.json SOURCES  ${standardoutputview_LIB_SRCS})

target_link_libraries(kdevstandardoutputview
    KDev::Interfaces
    KDev::Sublime
    KDev::Util
    KDev::OutputView
)

install( FILES kdevstandardoutputview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevstandardoutputview )

