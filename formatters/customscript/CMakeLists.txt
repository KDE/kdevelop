########### indent ###############
set(kdevcustomscript_PART_SRCS
    customscript_plugin.cpp
)

add_library(kdevcustomscript MODULE ${kdevcustomscript_PART_SRCS})
target_link_libraries(kdevcustomscript
    
    
    KDev::Interfaces
    KDev::Util)

install(TARGETS kdevcustomscript DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

########### install files ###############

configure_file(kdevcustomscript.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomscript.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomscript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
