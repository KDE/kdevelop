project(QMakeManager)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9024 )


find_package(KDevelop-PG-Qt QUIET)
include_directories(
    ${CMAKE_SOURCE_DIR}/plugins/builders/qmakebuilder
    ${QMakeManager_SOURCE_DIR}
    ${QMakeManager_BINARY_DIR}
)

add_subdirectory(parser)
add_subdirectory(duchain)

include_directories(${QMakeManager_SOURCE_DIR}/parser)

########### next target ###############

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectfile.cpp
    qmakeincludefile.cpp
    qmakemkspecs.cpp
    qmakefile.cpp
    qmakecache.cpp
    variablereferenceparser.cpp
)


kde4_add_plugin(kdevqmakemanager ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager ${KDE4_KDEUI_LIBS} ${KDevPlatform_INTERFACES_LIBRARY} ${KDevPlatform_PROJECT_LIBRARY} ${KDevPlatform_LANGUAGE_LIBRARY} kdev4qmakeparser )

########### install files ###############

install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

