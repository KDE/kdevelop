add_definitions(-DTRANSLATION_DOMAIN=\"kdevandroid\")

declare_qt_logging_category(androidplugin_SRCS
    TYPE PLUGIN
    HEADER debug_android.h
    IDENTIFIER ANDROID
    CATEGORY_BASENAME "android"
)
qt_add_resources(androidplugin_SRCS kdevandroidplugin.qrc)
ki18n_wrap_ui(androidplugin_SRCS androidpreferences.ui)
kconfig_add_kcfg_files(androidplugin_SRCS androidpreferencessettings.kcfgc)

kdevplatform_add_plugin(kdevandroid SOURCES androidplugin.cpp androidruntime.cpp androidpreferences.cpp ${androidplugin_SRCS})
target_link_libraries(kdevandroid
    KF6::CoreAddons
    KF6::KIOWidgets
    KDev::Interfaces
    KDev::Util
    KDev::OutputView
    KDev::Project
)
