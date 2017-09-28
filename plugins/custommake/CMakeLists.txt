add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustommake\")

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

ecm_qt_declare_logging_category(kdevcustommakemanager_PART_SRCS
    HEADER debug.h
    IDENTIFIER CUSTOMMAKE
    CATEGORY_NAME "kdevelop.plugins.custommake"
)
qt5_add_resources(kdevcustommakemanager_PART_SRCS kdevcustommakemanager.qrc)
kdevplatform_add_plugin(kdevcustommakemanager JSON kdevcustommakemanager.json SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::IMakeBuilder KDev::DefinesAndIncludesManager
    kdevmakefileresolver
)

add_subdirectory(makefileresolver)
