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

set(kdevkrossmanager_SRCS
    krossplugin.cpp
    krossbuildsystemmanager.cpp
    krossdistributedversioncontrol.cpp
    projectitemadaptors.cpp
    dvcsadaptors.cpp
    krossvcsjob.cpp
)

kde4_add_plugin(kdevkrossprojectmanager ${kdevkrossmanager_SRCS})
target_link_libraries(kdevkrossprojectmanager ${KDE4_KDEUI_LIBS}
	kdevplatforminterfaces kdevplatformproject kdevplatformvcs ${KDE4_KROSSUI_LIBS})

install(TARGETS kdevkrossprojectmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkrossmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})

