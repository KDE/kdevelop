add_definitions(-DTRANSLATION_DOMAIN=\"kdevfilemanager\")
########### next target ###############

set(kdevfilemanager_PART_SRCS
    kdevfilemanagerplugin.cpp
    filemanager.cpp
    bookmarkhandler.cpp
)

kdevplatform_add_plugin(kdevfilemanager JSON kdevfilemanager.json SOURCES ${kdevfilemanager_PART_SRCS})
target_link_libraries(kdevfilemanager KF5::KIOCore KF5::KIOWidgets KF5::TextEditor KF5::Solid KF5::KDELibs4Support KDev::Interfaces)

########### install files ###############

install( FILES kdevfilemanager.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevfilemanager )


