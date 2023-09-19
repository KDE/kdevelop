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
qt_add_resources(kdevcustommakemanager_PART_SRCS kdevcustommakemanager.qrc)
kdevplatform_add_plugin(KDevCustomMakeManager SOURCES ${kdevcustommakemanager_PART_SRCS})
target_link_libraries(KDevCustomMakeManager
    KF6::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::IMakeBuilder KDev::DefinesAndIncludesManager
    kdevmakefileresolver
)

add_subdirectory(makefileresolver)
