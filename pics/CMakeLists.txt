add_subdirectory(mini)
add_subdirectory(toolbar) 
add_subdirectory(misc) 

ecm_install_icons(ICONS
    hi16-apps-kdevelop.png hi32-apps-kdevelop.png hi48-apps-kdevelop.png
    DESTINATION ${ICON_INSTALL_DIR})
