project(kdev-qml)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_definitions(-DQT_CREATOR) # :D
add_definitions(-DQML_BUILD_STATIC_LIB=0 -DQMLJS_BUILD_DIR)

add_subdirectory(libs)
add_subdirectory(duchain)
add_subdirectory(tests)

kde4_add_plugin(kdevqmljslanguagesupport
    qmljsparsejob.cpp
    kdevqmljsplugin.cpp
)
target_link_libraries(kdevqmljslanguagesupport
    kdevqmljsduchain
    ${KDE4_THREADWEAVER_LIBS}
)

install(FILES kdevqmljssupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kdevqmljslanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
