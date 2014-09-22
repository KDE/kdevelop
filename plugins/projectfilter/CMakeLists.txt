##############################
# PLUGIN #####################
##############################

set( projectfilter_SRCS
    projectfilterprovider.cpp
    projectfilter.cpp
    projectfilterdebug.cpp
    filter.cpp
    projectfilterkcm.cpp
    projectfilterdebug.cpp
    filter.cpp
    filtermodel.cpp
    comboboxdelegate.cpp
)

ki18n_wrap_ui(projectfilter_SRCS projectfiltersettings.ui)
kconfig_add_kcfg_files(projectfilter_SRCS projectfiltersettings.kcfgc)

add_library( kdevprojectfilter MODULE ${projectfilter_SRCS} )
target_link_libraries(kdevprojectfilter KDev::Project KDev::Util KDev::Interfaces)
configure_file(kdevprojectfilter.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop)
kcoreaddons_desktop_to_json(kdevprojectfilter ${CMAKE_CURRENT_BINARY_DIR}/kdevprojectfilter.desktop)

install(TARGETS kdevprojectfilter DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

add_subdirectory(tests)
