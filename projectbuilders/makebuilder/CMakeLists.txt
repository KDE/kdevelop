project(makebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9037 )



########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    makebuilder.cpp
    makejob.cpp
)


add_library(kdevmakebuilder MODULE ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder
        ${KDE4_KDEUI_LIBS}
        KF5::TextEditor
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
)

install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


set( makebuilder_cfg_SRCS
    makebuilderpreferences.cpp
)

set( makebuilder_cfg_UI
    makeconfig.ui
)


qt5_wrap_ui( makebuilder_cfg_SRCS ${makebuilder_cfg_UI} )
kconfig_add_kcfg_files( makebuilder_cfg_SRCS makebuilderconfig.kcfgc )
add_library(kcm_kdev_makebuilder MODULE ${makebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_makebuilder KF5::KIOWidgets ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )

install(TARGETS kcm_kdev_makebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)

########### install files ###############
install( FILES imakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/make COMPONENT Devel)
configure_file(kcm_kdev_makebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_makebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_makebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})
configure_file(kdevmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES kdevmakebuilder.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevmakebuilder )
