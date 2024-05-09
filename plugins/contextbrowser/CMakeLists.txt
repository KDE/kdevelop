add_definitions(-DTRANSLATION_DOMAIN=\"kdevcontextbrowser\")

########### next target ###############

set(kdevcontextbrowser_PART_SRCS
    contextbrowser.cpp
    contextbrowserview.cpp
    browsemanager.cpp
)

declare_qt_logging_category(kdevcontextbrowser_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_CONTEXTBROWSER
    CATEGORY_BASENAME "contextbrowser"
)
qt_add_resources(kdevcontextbrowser_PART_SRCS kdevcontextbrowser.qrc)
kdevplatform_add_plugin(kdevcontextbrowser SOURCES ${kdevcontextbrowser_PART_SRCS})

target_link_libraries(kdevcontextbrowser KDev::Interfaces KDev::Util KDev::Language KDev::Sublime KDev::Shell KF6::TextEditor KF6::Parts)
