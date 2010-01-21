project(kdevelop4-custom-buildsystem)

find_package(KDE4 4.3.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KDevPlatform 0.9.97 REQUIRED)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR})

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

set( custom_SRCS custombuildsystemplugin.cpp )

kde4_add_plugin( kdevcustombuildsystem ${custom_SRCS} )
target_link_libraries( kdevcustombuildsystem ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES}  )

install( TARGETS kdevcustombuildsystem DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kdevcustombuildsystem.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

