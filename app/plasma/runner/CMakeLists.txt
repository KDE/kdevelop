kcoreaddons_add_plugin(krunner_kdevelopsessions SOURCES kdevelopsessions.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_kdevelopsessions
    KDevelopSessionsWatch
    KF6::I18n
    KF6::Runner
)
