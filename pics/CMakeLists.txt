########### install files ###############

set( kdevplatform_icon_dirs  22x22  )

install( DIRECTORY ${kdevplatform_icon_dirs} DESTINATION ${ICON_INSTALL_DIR}/hicolor PATTERN .svn EXCLUDE)

