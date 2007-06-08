
# add_subdirectory(tests)

include_directories(${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/buildtools/builders/makebuilder
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
    custommaketreesynchronizer.cpp
)

kde4_automoc(${kdevcustommakemanager_PART_SRCS})
kde4_add_plugin(kdevcustommakemanager ${kdevcustommakemanager_PART_SRCS})
target_link_libraries( kdevcustommakemanager
                    ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY}
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcustommakemanager )
