add_subdirectory(projectfileplasmoid)

install(DIRECTORY branches/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kdevelop.branches)
install(FILES branches/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME kdevelop-dashboard-branches.desktop)
