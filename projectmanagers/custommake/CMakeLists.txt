add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustommake\")

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

kdevplatform_add_plugin(kdevcustommakemanager JSON kdevcustommakemanager.json SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::IMakeBuilder
    kdevmakefileresolver
)


########### install files ###############

install( FILES kdevcustommakemanager.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcustommakemanager )

add_subdirectory(makefileresolver)
