add_definitions(-DTRANSLATION_DOMAIN=\"kdevstandardoutputview\")

########### next target ###############

declare_qt_logging_category(standardoutputview_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_STANDARDOUTPUTVIEW
    CATEGORY_BASENAME "standardoutputview"
)

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
    standardoutputviewmetadata.cpp
    ${standardoutputview_LOG_PART_SRCS}
)

kdevplatform_add_plugin(kdevstandardoutputview JSON kdevstandardoutputview.json SOURCES  ${standardoutputview_LIB_SRCS})

target_link_libraries(kdevstandardoutputview
    KDev::Interfaces
    KDev::Sublime
    KDev::Util
    KDev::OutputView
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
