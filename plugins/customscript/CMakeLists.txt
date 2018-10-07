add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustomscript\")
set(kdevcustomscript_PART_SRCS
    customscript_plugin.cpp
)

declare_qt_logging_category(kdevcustomscript_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER CUSTOMSCRIPT
    CATEGORY_BASENAME "customscript"
)
kdevplatform_add_plugin(kdevcustomscript JSON kdevcustomscript.json SOURCES ${kdevcustomscript_PART_SRCS})
target_link_libraries(kdevcustomscript
    KDev::Interfaces
    KDev::Util)
