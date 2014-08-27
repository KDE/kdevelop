set(krunner_kdevelopsessions_SRCS
    kdevelopsessions.cpp
)

add_library(krunner_kdevelopsessions MODULE ${krunner_kdevelopsessions_SRCS})
target_link_libraries(krunner_kdevelopsessions KF5::Plasma KF5::KIOWidgets KF5::KDELibs4Support)

install(TARGETS krunner_kdevelopsessions DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES kdevelopsessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})

