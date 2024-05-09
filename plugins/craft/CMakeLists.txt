add_definitions(-DTRANSLATION_DOMAIN=\"kdevcraft\")

declare_qt_logging_category(craftplugin_LOG_SRCS
    TYPE PLUGIN
    HEADER debug_craft.h
    IDENTIFIER CRAFT
    CATEGORY_BASENAME "craft"
)

#qt_add_resources(craftplugin_SRCS kdevcraftplugin.qrc)
kdevplatform_add_plugin(kdevcraft SOURCES craftplugin.cpp craftruntime.cpp ${craftplugin_LOG_SRCS})
target_link_libraries(kdevcraft
    KF6::CoreAddons
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
