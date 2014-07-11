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

add_library(kdev4includesdefinessettings settingsmanager.cpp compilerprovider/icompiler.cpp)
target_link_libraries( kdev4includesdefinessettings LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES} )

add_library(kdevdefinesandincludesmanager MODULE
        ${kdevdefinesandincludesmanager_SRCS} )
target_link_libraries( kdevdefinesandincludesmanager LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
        kdev4includesdefinessettings
        kdev4noprojectincludesanddefines )

install( TARGETS kdevdefinesandincludesmanager
         DESTINATION ${PLUGIN_INSTALL_DIR} )
install( TARGETS kdev4includesdefinessettings ${INSTALL_TARGETS_DEFAULT_ARGS} )

configure_file(kdevdefinesandincludesmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevdefinesandincludesmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevdefinesandincludesmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES
    idefinesandincludesmanager.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/custom-definesandincludes COMPONENT Devel
)
