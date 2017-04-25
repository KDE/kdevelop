add_definitions(-DTRANSLATION_DOMAIN=\"kdevflatpak\")

qt5_add_resources(flatpakplugin_SRCS kdevflatpakplugin.qrc)
kdevplatform_add_plugin(kdevflatpak JSON kdevflatpak.json SOURCES flatpakplugin.cpp flatpakruntime.cpp ${flatpakplugin_SRCS})
target_link_libraries(kdevflatpak
    KF5::CoreAddons
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)
