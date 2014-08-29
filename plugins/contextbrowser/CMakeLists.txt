
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9524 )


########### next target ###############

set(kdevcontextbrowser_PART_SRCS
    contextbrowser.cpp
    contextbrowserview.cpp
    browsemanager.cpp
)

add_library(kdevcontextbrowser MODULE ${kdevcontextbrowser_PART_SRCS})
target_link_libraries(kdevcontextbrowser KDev::Interfaces KDev::Util KDev::Language KDev::Sublime KF5::TextEditor KF5::Parts)

install(TARGETS kdevcontextbrowser DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(FILES kdevcontextbrowser.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcontextbrowser)

########### install files ###############

configure_file(kdevcontextbrowser.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcontextbrowser.desktop)
kcoreaddons_desktop_to_json(kdevcontextbrowser ${CMAKE_CURRENT_BINARY_DIR}/kdevcontextbrowser.desktop)
