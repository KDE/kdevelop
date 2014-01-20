
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9517 )

add_subdirectory(test)

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

add_library(kdevgenericmanager MODULE ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager
    KF5::KIOWidgets
    kdevplatforminterfaces kdevplatformproject
)

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevgenericmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
