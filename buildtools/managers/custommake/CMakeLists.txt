
# add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/buildtools/builders/makebuilder
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
    projectfilesystemwatcher.cpp
)

kde4_automoc(${kdevcustommakemanager_PART_SRCS})
kde4_add_plugin(kdevcustommakemanager ${kdevcustommakemanager_PART_SRCS})
target_link_libraries( kdevcustommakemanager
                    ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcustommakemanager )
