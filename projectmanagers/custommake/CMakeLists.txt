project(custommakemanager)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9025 )
# add_subdirectory(tests)

include_directories(
    ${makebuilder_SOURCE_DIR}
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)


add_library(kdevcustommakemanager MODULE ${kdevcustommakemanager_PART_SRCS})
target_link_libraries( kdevcustommakemanager
                    ${KDE4_KDEUI_LIBS} KF5::KIOWidgets ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevcustommakemanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcustommakemanager )
