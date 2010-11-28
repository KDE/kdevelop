project(kdevelop4-custom-buildsystem)

set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

find_package(KDE4 4.4.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KDevPlatform 1.1.0 REQUIRED)

if(KDevPlatform_VERSION VERSION_GREATER 1.1.50)
    MESSAGE(FATAL_ERROR "This version of the plugin doesn't work with KDevPlatform 1.2, version 1.1 is required")
endif(KDevPlatform_VERSION VERSION_GREATER 1.1.50)


include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR})

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
    )

## KCM for the Plugin
set( custom_kcm_SRCS 
        projectpathsmodel.cpp 
        definesmodel.cpp
        includesmodel.cpp
        configwidget.cpp
        custombuildsystemconfigwidget.cpp
        kcm_custombuildsystem.cpp 
   )
kde4_add_ui_files( custom_kcm_SRCS configwidget.ui custombuildsystemconfigwidget.ui )
kde4_add_kcfg_files( custom_kcm_SRCS kcfg_custombuildsystemconfig.kcfgc )
kde4_add_plugin( kcm_kdevcustombuildsystem ${custom_kcm_SRCS} )
target_link_libraries( kcm_kdevcustombuildsystem ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} )

## Installation
install( TARGETS kcm_kdevcustombuildsystem
                 kdevcustombuildsystem
         DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kdevcustombuildsystem.desktop
               kcm_kdevcustombuildsystem.desktop
         DESTINATION ${SERVICES_INSTALL_DIR} )

