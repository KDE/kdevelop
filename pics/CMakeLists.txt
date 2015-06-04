########### install files ###############

set( kdevplatform_icon_dirs  22x22  )

install( DIRECTORY ${kdevplatform_icon_dirs} DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor PATTERN .svn EXCLUDE)

