project(kdevelop4-custom-buildsystem)

## Unittests
add_subdirectory( tests )

## KDevelop Plugin
set( custom_SRCS custombuildsystemplugin.cpp custombuildjob.cpp )
add_library(kdevcustombuildsystem MODULE ${custom_SRCS} )
target_link_libraries( kdevcustombuildsystem
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    )

## KCM for the Plugin
set( custom_kcm_SRCS
        configwidget.cpp
        custombuildsystemconfigwidget.cpp
        kcm_custombuildsystem.cpp
        debugarea.cpp
   )
qt5_wrap_ui( custom_kcm_SRCS 
                        configwidget.ui
                        custombuildsystemconfigwidget.ui
                 )
kconfig_add_kcfg_files( custom_kcm_SRCS kcfg_custombuildsystemconfig.kcfgc )
add_library(kcm_kdevcustombuildsystem MODULE ${custom_kcm_SRCS} )
target_link_libraries( kcm_kdevcustombuildsystem KF5::KIOWidgets ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )

## Installation
install(TARGETS kcm_kdevcustombuildsystem DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)
install(TARGETS kdevcustombuildsystem DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

configure_file(kdevcustombuildsystem.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop)
configure_file(kcm_kdevcustombuildsystem.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcustombuildsystem.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop
              ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcustombuildsystem.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
