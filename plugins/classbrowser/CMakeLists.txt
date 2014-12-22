add_definitions(-DTRANSLATION_DOMAIN=\"kdevclassbrowser\")
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

kdevplatform_add_plugin(kdevclassbrowser JSON kdevclassbrowser.json SOURCES ${kdevclassbrowser_PART_SRCS})

target_link_libraries(kdevclassbrowser KF5::TextEditor KDev::Util KDev::Language KDev::Interfaces KDev::Project)

########### install files ###############

install(FILES kdevclassbrowser.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevclassbrowser)
