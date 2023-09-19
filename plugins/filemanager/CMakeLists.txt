add_definitions(-DTRANSLATION_DOMAIN=\"kdevfilemanager\")

set(kdevfilemanager_PART_SRCS
    kdevfilemanagerplugin.cpp
    filemanager.cpp
    bookmarkhandler.cpp
)
declare_qt_logging_category(kdevfilemanager_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_FILEMANAGER
    CATEGORY_BASENAME "filemanager"
)
qt_add_resources(kdevfilemanager_PART_SRCS kdevfilemanager.qrc)
kdevplatform_add_plugin(kdevfilemanager SOURCES ${kdevfilemanager_PART_SRCS})
target_link_libraries(kdevfilemanager
    KDev::Interfaces
    KDev::Sublime
    KF${KF_MAJOR_VERSION}::Bookmarks
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::KIOFileWidgets
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::TextEditor
)
