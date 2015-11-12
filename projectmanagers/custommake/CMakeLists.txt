add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustommake\")

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

qt5_add_resources(kdevcustommakemanager_PART_SRCS kdevcustommakemanager.qrc)
kdevplatform_add_plugin(kdevcustommakemanager JSON kdevcustommakemanager.json SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::IMakeBuilder
    kdevmakefileresolver
)

add_subdirectory(makefileresolver)
