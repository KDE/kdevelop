include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_subdirectory(kcm_widget)
add_subdirectory(tests)
add_subdirectory(compilerprovider)
add_subdirectory(noprojectincludesanddefines)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set( kdevdefinesandincludesmanager_SRCS
        definesandincludesmanager.cpp
        debugarea.cpp
    )

add_library(kdevdefinesandincludessettings settingsmanager.cpp compilerprovider/icompiler.cpp)
target_link_libraries( kdevdefinesandincludessettings LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES} )
generate_export_header(kdevdefinesandincludessettings EXPORT_FILE_NAME definesandincludessettingsexport.h)

add_library(kdevdefinesandincludesmanager MODULE
        ${kdevdefinesandincludesmanager_SRCS} )
target_link_libraries( kdevdefinesandincludesmanager LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
        kdevdefinesandincludessettings
        kdev4noprojectincludesanddefines )
generate_export_header(kdevdefinesandincludesmanager EXPORT_FILE_NAME definesandincludesmanagerexport.h)

install( TARGETS kdevdefinesandincludesmanager
         DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )
install( TARGETS kdevdefinesandincludessettings ${INSTALL_TARGETS_DEFAULT_ARGS} )

configure_file(kdevdefinesandincludesmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevdefinesandincludesmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevdefinesandincludesmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES
    idefinesandincludesmanager.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/custom-definesandincludes COMPONENT Devel
)
