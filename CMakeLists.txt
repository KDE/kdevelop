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
add_subdirectory(tests)

kde4_add_plugin(kdev-qmljsplugin duchain/contextbuilder.cpp duchain/typebuilder.cpp duchain/declarationbuilder.cpp parseqmljsjob.cpp kdevqmljsplugin.cpp)
target_link_libraries(kdev-qmljsplugin qtc-qmlsupport
    ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_THREADWEAVER_LIBS})

install(FILES kdevqmljssupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kdev-qmljsplugin DESTINATION ${PLUGIN_INSTALL_DIR})