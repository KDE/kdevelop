project(custommakemanager)

include_directories(
    ${makebuilder_SOURCE_DIR}
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

add_library(kdevcustommakemanager MODULE ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets KDev::Interfaces KDev::Project KDev::Util KDev::Language
    kdevmakefileresolver
)

install(TARGETS kdevcustommakemanager DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevcustommakemanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustommakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcustommakemanager.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcustommakemanager )

add_subdirectory(makefileresolver)
