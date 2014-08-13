
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9518 )

########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleviewplugin.cpp
    kdevkonsoleview.cpp
)

add_library(kdevkonsoleview MODULE ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF5::Parts KDev::Interfaces)

install(TARGETS kdevkonsoleview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevkonsoleview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevkonsoleview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevkonsoleview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

