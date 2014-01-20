##############################
# PLUGIN #####################
##############################

set( projectfilter_SRCS
    projectfilterprovider.cpp
    projectfilter.cpp
    projectfilterdebug.cpp
    filter.cpp
)

add_library( kdevprojectfilter MODULE ${projectfilter_SRCS} )
target_link_libraries( kdevprojectfilter
    KF5::KCMUtils
    kdevplatformproject kdevplatformutil kdevplatforminterfaces)

install( TARGETS kdevprojectfilter DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevprojectfilter.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

##############################
# KCM ########################
##############################

set( projectfilterkcm_SRCS
    projectfilterkcm.cpp
    projectfilterdebug.cpp
    filter.cpp
    filtermodel.cpp
    comboboxdelegate.cpp
)

set( projectfilterkcm_UI
    projectfiltersettings.ui
)

qt5_wrap_ui( projectfilterkcm_SRCS ${projectfilterkcm_UI} )
kconfig_add_kcfg_files( projectfilterkcm_SRCS projectfiltersettings.kcfgc )
add_library( kcm_kdevprojectfilter MODULE ${projectfilterkcm_SRCS} )
target_link_libraries( kcm_kdevprojectfilter
    KF5::KCMUtils
    kdevplatformproject kdevplatformutil kdevplatforminterfaces)

install( TARGETS kcm_kdevprojectfilter DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdevprojectfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(tests)
