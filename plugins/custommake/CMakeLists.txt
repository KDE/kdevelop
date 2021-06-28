add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustommake\")

set(kdevcustommakemanager_PART_SRCS
    custommakemanager.cpp
    custommakemodelitems.cpp
)

declare_qt_logging_category(kdevcustommakemanager_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER CUSTOMMAKE
    CATEGORY_BASENAME "custommake"
)
qt5_add_resources(kdevcustommakemanager_PART_SRCS kdevcustommakemanager.qrc)
kdevplatform_add_plugin(kdevcustommakemanager SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(kdevcustommakemanager
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::IMakeBuilder KDev::DefinesAndIncludesManager
    kdevmakefileresolver
)

add_subdirectory(makefileresolver)
