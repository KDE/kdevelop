add_definitions(-DTRANSLATION_DOMAIN=\"kdevstandardoutputview\")
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
    standardoutputviewmetadata.cpp
)

qt5_add_resources(standardoutputview_LIB_SRCS kdevstandardoutputview.qrc)
kdevplatform_add_plugin(kdevstandardoutputview JSON kdevstandardoutputview.json SOURCES  ${standardoutputview_LIB_SRCS})

target_link_libraries(kdevstandardoutputview
    KDev::Interfaces
    KDev::Sublime
    KDev::Util
    KDev::OutputView
)

