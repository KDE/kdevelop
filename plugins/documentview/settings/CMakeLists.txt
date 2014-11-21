
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
install(TARGETS kcm_documentview_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcm_documentview_settings.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentview )
