########### indent ###############
set(kdevcustomscript_PART_SRCS
    customscript_plugin.cpp
)

add_library(kdevcustomscript MODULE ${kdevcustomscript_PART_SRCS})
target_link_libraries(kdevcustomscript
    
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kdevcustomscript DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

configure_file(kdevcustomscript.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomscript.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomscript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
