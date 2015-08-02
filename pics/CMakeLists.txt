add_subdirectory(mini)
add_subdirectory(toolbar) 
add_subdirectory(misc) 

ecm_install_icons(ICONS
    16-apps-kdevelop.png 32-apps-kdevelop.png 48-apps-kdevelop.png
    DESTINATION ${ICON_INSTALL_DIR})
