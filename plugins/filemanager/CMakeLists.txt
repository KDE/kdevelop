add_definitions(-DTRANSLATION_DOMAIN=\"kdevfilemanager\")

set(kdevfilemanager_PART_SRCS
    kdevfilemanagerplugin.cpp
    filemanager.cpp
    bookmarkhandler.cpp
)
kdevplatform_add_plugin(kdevfilemanager JSON kdevfilemanager.json SOURCES ${kdevfilemanager_PART_SRCS})
target_link_libraries(kdevfilemanager KF5::Bookmarks KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::TextEditor KDev::Interfaces)

install( FILES kdevfilemanager.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevfilemanager )
