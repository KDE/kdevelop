add_definitions(-DTRANSLATION_DOMAIN=\"kdevcodeutils\")

########### install target ###############

set(kdevcodeutils_PART_SRCS
    codeutilsplugin.cpp
)

declare_qt_logging_category(kdevcodeutils_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_CODEUTILS
    CATEGORY_BASENAME "codeutils"
)
qt_add_resources(kdevcodeutils_PART_SRCS kdevcodeutils.qrc)
kdevplatform_add_plugin(kdevcodeutils SOURCES ${kdevcodeutils_PART_SRCS})

target_link_libraries(kdevcodeutils
    KF${KF_MAJOR_VERSION}::Parts
    KF${KF_MAJOR_VERSION}::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)
