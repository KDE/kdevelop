# install the cmake files

set(cmakeFiles FindKDevPlatform.cmake)
install( FILES ${cmakeFiles} DESTINATION ${DATA_INSTALL_DIR}/cmake/modules )

install( FILES
        KDevPlatformMacros.cmake
        DESTINATION "${_kdevplatform_CONFIG_DEST}" )
