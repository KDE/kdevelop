add_subdirectory(kcm_widget)
add_subdirectory(tests)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} )

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set( kdevcustomdefinesandincludesmanager_SRCS
        customdefinesandincludesmanager.cpp
        settingsmanager.cpp
        settingsconverter.cpp
    )
kde4_add_plugin( kdevcustomdefinesandincludesmanager
        ${kdevcustomdefinesandincludesmanager_SRCS})
target_link_libraries( kdevcustomdefinesandincludesmanager LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES} )

install( TARGETS kdevcustomdefinesandincludesmanager
         DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevcustomdefinesandincludesmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomdefinesandincludesmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustomdefinesandincludesmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
