add_definitions(-DTRANSLATION_DOMAIN=\"kdevcontextbrowser\")

########### next target ###############

set(kdevcontextbrowser_PART_SRCS
    contextbrowser.cpp
    contextbrowserview.cpp
    browsemanager.cpp
)

kdevplatform_add_plugin(kdevcontextbrowser JSON kdevcontextbrowser.json SOURCES ${kdevcontextbrowser_PART_SRCS})

target_link_libraries(kdevcontextbrowser KDev::Interfaces KDev::Util KDev::Language KDev::Sublime KF5::TextEditor KF5::Parts)

install(FILES kdevcontextbrowser.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcontextbrowser)
