set(krunner_kdevelopsessions_SRCS
    kdevelopsessions.cpp
)

kde4_add_plugin(krunner_kdevelopsessions ${krunner_kdevelopsessions_SRCS})
target_link_libraries(krunner_kdevelopsessions ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_kdevelopsessions DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES kdevelopsessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})

