project(krosssupport)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/vcs/interfaces
    ${CMAKE_SOURCE_DIR}/vcs
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(kdevkrossplugin_SRCS
    krossplugin.cpp
    krossbuildsystemmanager.cpp
    krossdistributedversioncontrol.cpp
    krosstoolviewfactory.cpp
    krossvcsjob.cpp
    projectitemadaptors.cpp
    dvcsadaptors.cpp
)

kde4_add_plugin(kdevkrossplugin ${kdevkrossplugin_SRCS})
target_link_libraries(kdevkrossplugin ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces kdevplatformproject kdevplatformvcs ${KDE4_KROSSUI_LIBS})

install(TARGETS kdevkrossplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkrossplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

