
### Configuration module for general kdevelop settings

set( documentview_config_SRC
preferences.cpp
)
set( documentview_config_UI
settingswidget.ui
)
ki18n_wrap_ui( documentview_config_SRC ${documentview_config_UI} )

kconfig_add_kcfg_files(documentview_config_SRC config.kcfgc )
add_library(kcm_documentview_settings MODULE ${documentview_config_SRC})

target_link_libraries(kcm_documentview_settings KF5::KCMUtils kdevplatform )
install(TARGETS kcm_documentview_settings  DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES kcm_documentview_settings.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
