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
    KF6::Parts
    KF6::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)
