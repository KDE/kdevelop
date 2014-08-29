
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9517 )

add_subdirectory(test)

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

add_library(kdevgenericmanager MODULE ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager
    KF5::KIOWidgets
    KDev::Project
)

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevgenericmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop)
kcoreaddons_desktop_to_json(kdevgenericmanager ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop)
