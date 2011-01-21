project(QMakeManager)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${QMakeManager_SOURCE_DIR}/cmake/)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9024 )

find_package(KDE4 4.2.0 REQUIRED)
find_package(KDevPlatform 0.9.93 REQUIRED)
find_package(KDevelop REQUIRED)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} ${KDEVELOP_INCLUDE_DIR} )

add_subdirectory(qmakebuilder)
add_subdirectory(app_templates)

find_package(KDevelop-PG-Qt REQUIRED)
include_directories(
    ${qmakebuilder_SOURCE_DIR}
    ${QMakeManager_SOURCE_DIR}
    ${QMakeManager_BINARY_DIR}
)

add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(tests)

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
    qmakejob.cpp
    qmakebuilddirchooser.cpp
)

set(kdevqmakemanager_PART_UI
    qmakebuilddirchooser.ui
)

kde4_add_ui_files( kdevqmakemanager_PART_SRCS ${kdevqmakemanager_PART_UI} )

kde4_add_plugin(kdevqmakemanager ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    kdev4qmakeparser )

########### install files ###############

install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

