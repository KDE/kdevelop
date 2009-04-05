
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9512 )

########### next target ###############

set(kdevclassbrowser_PART_SRCS
    classbrowserplugin.cpp
    classmodel.cpp
    classmodelnode.cpp
    classmodelnodescontroller.cpp
    classtree.cpp
    allclassesfolder.cpp
)

kde4_add_plugin(kdevclassbrowser ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdevplatformlanguage kdevplatforminterfaces kdevplatformproject)

install(TARGETS kdevclassbrowser DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevclassbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevclassbrowser.rc DESTINATION ${DATA_INSTALL_DIR}/kdevclassbrowser)
