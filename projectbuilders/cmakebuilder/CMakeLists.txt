project(cmakebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9032 )
include_directories(
    ${cmakebuilder_SOURCE_DIR}
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    )


########### next target ###############

set(cmakebuilder_LIB_SRCS
    cmakebuilder.cpp
    cmakejob.cpp
    prunejob.cpp
)

kconfig_add_kcfg_files( cmakebuilder_LIB_SRCS cmakebuilderconfig.kcfgc )
add_library(kdevcmakebuilder MODULE ${cmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevcmakebuilder
    kdevcmakecommon
        KF5::KIOWidgets
        ${KDE4_KDEUI_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevcmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevcmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
########### next target ###############

set( cmakebuilder_cfg_SRCS
    cmakebuilderpreferences.cpp
)

set( cmakebuilder_cfg_UI
    cmakebuildersettings.ui
)


qt5_wrap_ui( cmakebuilder_cfg_SRCS ${cmakebuilder_cfg_UI} )
kconfig_add_kcfg_files( cmakebuilder_cfg_SRCS cmakebuilderconfig.kcfgc )

add_library(kcm_kdev_cmakebuilder MODULE ${cmakebuilder_cfg_SRCS} )
target_link_libraries(kcm_kdev_cmakebuilder KF5::KIOWidgets ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )
configure_file(kcm_kdev_cmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_cmakebuilder.desktop)
install(TARGETS kcm_kdev_cmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_cmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})


