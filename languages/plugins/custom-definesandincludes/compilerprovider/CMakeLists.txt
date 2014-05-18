set( compilerprovider_SRCS
        compilerprovider.cpp
        gcclikeprovider.cpp
        msvcprovider.cpp
        ../debugarea.cpp
    )

kde4_add_plugin( kdevcompilerprovider
        ${compilerprovider_SRCS})

target_link_libraries( kdevcompilerprovider LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
        kdev4includesdefinessettings )

install( TARGETS kdevcompilerprovider
         DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevcompilerprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcompilerprovider.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcompilerprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

option(BUILD_kdev_msvcdefinehelper "Build the msvcdefinehelper tool for retrieving msvc standard macro definitions" OFF)

if(BUILD_kdev_msvcdefinehelper)
    set(kdevmsvcdefinehelper_SRCS msvcdefinehelper.cpp)
    add_executable(kdevmsvcdefinehelper ${kdevmsvcdefinehelper_SRCS})
    install(TARGETS kdevmsvcdefinehelper RUNTIME DESTINATION bin
                                         LIBRARY DESTINATION lib)
endif(BUILD_kdev_msvcdefinehelper)