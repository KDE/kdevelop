set(krunner_kdevelopsessions_SRCS
    kdevelopsessions.cpp
)

add_library(krunner_kdevelopsessions MODULE ${krunner_kdevelopsessions_SRCS})
target_link_libraries(krunner_kdevelopsessions
    KDevelopSessionsWatch
    KF5::I18n
    KF5::Runner
)

install(TARGETS krunner_kdevelopsessions DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner )

kcoreaddons_desktop_to_json(krunner_kdevelopsessions kdevelopsessions.desktop)
