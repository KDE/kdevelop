
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9025 )
# add_subdirectory(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/builders/makebuilder
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
    projectfilesystemwatcher.cpp
    custommaketreesynchronizer.cpp
)


kde4_add_plugin(kdevcustommakemanager ${kdevcustommakemanager_PART_SRCS})
target_link_libraries( kdevcustommakemanager
                    ${KDE4_KDEUI_LIBS} ${KDevPlatform_INTERFACES_LIBRARY} ${KDevPlatform_PROJECT_LIBRARY} ${KDevPlatform_LANGUAGE_LIBRARY}
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcustommakemanager )
