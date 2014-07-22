
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDevPlatform_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# workaround a boost bug in 1.37 and 1.38 that causes link failure when exceptions are disabled
# see https://svn.boost.org/trac/boost/ticket/2947 for details
if( ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_PATCH_VERSION} VERSION_GREATER 1.36.1 )
    add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
endif()

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9512 )

########### next target ###############

set(kdevclassbrowser_PART_SRCS
    classbrowserplugin.cpp
    classmodel.cpp
    classmodelnode.cpp
    classmodelnodescontroller.cpp
    classwidget.cpp
    classtree.cpp
    documentclassesfolder.cpp
    projectfolder.cpp
    allclassesfolder.cpp
)

add_library(kdevclassbrowser MODULE ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser KF5::TextEditor kdevplatformutil kdevplatformlanguage kdevplatforminterfaces kdevplatformproject)

install(TARGETS kdevclassbrowser DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevclassbrowser.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevclassbrowser.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevclassbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevclassbrowser.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevclassbrowser)
