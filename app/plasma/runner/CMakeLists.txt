kcoreaddons_add_plugin(krunner_kdevelopsessions SOURCES kdevelopsessions.cpp INSTALL_NAMESPACE "kf6/krunner")
set_target_properties(krunner_kdevelopsessions PROPERTIES
    # Set the plugin file base name, which determines the plugin ID, to "kdevelopsessions".
    # This was the Plasma 5 ID of this plugin previously specified in kdevelopsessions.desktop.
    # The plugin ID should be stable in order to keep its enabled state possibly configured by users.
    OUTPUT_NAME "kdevelopsessions"
)
target_link_libraries(krunner_kdevelopsessions
    KDevelopSessionsWatch
    KF6::I18n
    KF6::Runner
)
