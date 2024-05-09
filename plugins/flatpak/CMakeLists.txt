kdevplatform_add_file_templates(flatpak-template)

add_definitions(-DTRANSLATION_DOMAIN=\"kdevflatpak\")

declare_qt_logging_category(flatpakplugin_SRCS
    TYPE PLUGIN
    HEADER debug_flatpak.h
    IDENTIFIER FLATPAK
    CATEGORY_BASENAME "flatpak"
)

qt_add_resources(flatpakplugin_SRCS kdevflatpakplugin.qrc)
kdevplatform_add_plugin(kdevflatpak SOURCES flatpakplugin.cpp flatpakruntime.cpp ${flatpakplugin_SRCS})
target_link_libraries(kdevflatpak
    KF6::CoreAddons
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)

install(FILES replicate.sh DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kdevelop/kdevflatpak/)
