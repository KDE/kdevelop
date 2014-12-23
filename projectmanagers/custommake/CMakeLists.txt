project(custommakemanager)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustommake\")
include_directories(
    ${makebuilder_SOURCE_DIR}
    )


########### next target ###############

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

kdevplatform_add_plugin(kdevcustommakemanager JSON kdevcustommakemanager.json SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets KDev::Interfaces KDev::Project KDev::Util KDev::Language
    kdevmakefileresolver
)


########### install files ###############

install( FILES kdevcustommakemanager.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcustommakemanager )

add_subdirectory(makefileresolver)
