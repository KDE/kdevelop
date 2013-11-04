project(kdev-ninja)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

find_package(KDevPlatform REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
)


########### next target ###############

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

kde4_add_plugin(kdevninja ninjajob.cpp kdevninjabuilderplugin.cpp)
target_link_libraries(kdevninja
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
)

install(TARGETS kdevninja DESTINATION ${PLUGIN_INSTALL_DIR})


########### configurator ##############

kde4_add_ui_files(ninjabuilder_cfg_SRCS ninjaconfig.ui)
kde4_add_kcfg_files(ninjabuilder_cfg_SRCS ninjabuilderconfig.kcfgc)
kde4_add_plugin(kcm_kdev_ninjabuilder ninjabuilderpreferences.cpp ${ninjabuilder_cfg_SRCS})
target_link_libraries(kcm_kdev_ninjabuilder
	${KDE4_KIO_LIBS}
	${KDEVPLATFORM_INTERFACES_LIBRARIES}
	${KDEVPLATFORM_PROJECT_LIBRARIES}
	${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kcm_kdev_ninjabuilder DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_kdev_ninjabuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})

configure_file(kdevninja.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevninja.desktop DESTINATION ${SERVICES_INSTALL_DIR})


