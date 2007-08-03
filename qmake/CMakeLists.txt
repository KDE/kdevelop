
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KDevPlatform REQUIRED)
find_package(KDevelop REQUIRED)
find_package(KDevelop-PG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES}
	${KDEVELOP_INCLUDE_DIR}/qmake/
	)

include_directories(${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    )

add_subdirectory(parser)

include_directories(${CMAKE_SOURCE_DIR}/parser)

########### next target ###############

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectfile.cpp
    qmakeincludefile.cpp
    qmakemkspecs.cpp
    qmakefile.cpp
)


kde4_add_plugin(kdevqmakemanager ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} kdevqmakeparser )

########### install files ###############

install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )

