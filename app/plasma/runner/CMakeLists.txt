kcoreaddons_add_plugin(krunner_kdevelopsessions SOURCES kdevelopsessions.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_kdevelopsessions
    KDevelopSessionsWatch
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Runner
)
