project(kdevelop4-custom-buildsystem)

include(MacroLibrary)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} ${customDefinesAndIncludes_INCLUDE_DIR})

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

## Unittests
add_subdirectory( tests )

## KDevelop Plugin
set( custom_SRCS custombuildsystemplugin.cpp custombuildjob.cpp )
kde4_add_plugin( kdevcustombuildsystem ${custom_SRCS} )
target_link_libraries( kdevcustombuildsystem
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        kdevcustomdefinesandincludesmanager
    )

## KCM for the Plugin
set( custom_kcm_SRCS
        configwidget.cpp
        custombuildsystemconfigwidget.cpp
        kcm_custombuildsystem.cpp
        debugarea.cpp
   )
kde4_add_ui_files( custom_kcm_SRCS
                        configwidget.ui
                        custombuildsystemconfigwidget.ui
                 )
kde4_add_kcfg_files( custom_kcm_SRCS kcfg_custombuildsystemconfig.kcfgc )
kde4_add_plugin( kcm_kdevcustombuildsystem ${custom_kcm_SRCS} )
target_link_libraries( kcm_kdevcustombuildsystem ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )

## Installation
install( TARGETS kcm_kdevcustombuildsystem
                 kdevcustombuildsystem
         DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevcustombuildsystem.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop
               kcm_kdevcustombuildsystem.desktop
         DESTINATION ${SERVICES_INSTALL_DIR} )
