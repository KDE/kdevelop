project(kdevelopsessions)

install(DIRECTORY package/
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/kdevelopsessions)

install(FILES package/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-applet-kdevelopsessions.desktop)
