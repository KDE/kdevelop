project(custommakemanager)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9025 )
# add_subdirectory(tests)

include_directories(
    ${makebuilder_SOURCE_DIR}
    ${customDefinesAndIncludes_INCLUDE_DIR}
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)


kde4_add_plugin(kdevcustommakemanager ${kdevcustommakemanager_PART_SRCS})
target_link_libraries( kdevcustommakemanager
                    ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
                    kdevcustomdefinesandincludesmanager
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevcustommakemanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcustommakemanager )
