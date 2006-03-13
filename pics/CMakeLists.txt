kde4_header()

add_subdirectory(mini)
add_subdirectory(toolbar) 
add_subdirectory(misc) 


########### install files ###############

kde4_install_icons(${ICON_INSTALL_DIR} hicolor)


kde4_footer()

