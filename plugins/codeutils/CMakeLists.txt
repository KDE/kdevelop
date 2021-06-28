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
qt5_add_resources(kdevcodeutils_PART_SRCS kdevcodeutils.qrc)
kdevplatform_add_plugin(kdevcodeutils SOURCES ${kdevcodeutils_PART_SRCS})

target_link_libraries(kdevcodeutils
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)
