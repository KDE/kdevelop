project(QMakeManager)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)
find_package(KDevelop REQUIRED)

find_package(KDevelop-PG-Qt QUIET)
include_directories(
        ${QMakeManager_SOURCE_DIR} 
        ${QMakeManager_BINARY_DIR} 
        ${KDE4_INCLUDES}
        ${KDEVPLATFORM_INCLUDE_DIR}
        ${KDEVELOP_INCLUDE_DIR}
        )

add_subdirectory(parser)
add_subdirectory(duchain)

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
target_link_libraries( kdevqmakemanager ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} kdev4qmakeparser )

########### install files ###############

install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

