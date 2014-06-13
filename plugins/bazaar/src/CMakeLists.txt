
set(kdevbazaar_PART_SRCS
    diffjob.cpp
    bzrannotatejob.cpp
    bazaarplugin.cpp
    bazaarutils.cpp
    copyjob.cpp
)

kde4_add_plugin(kdevbazaar ${kdevbazaar_PART_SRCS})

target_link_libraries(kdevbazaar
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_VCS_LIBRARIES}
)

install(TARGETS kdevbazaar DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############
configure_file(kdevbazaar.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevbazaar.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevbazaar.desktop DESTINATION ${SERVICES_INSTALL_DIR} )