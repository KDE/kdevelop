set(kdevelopsessionsengine_SRCS
    kdevelopsessionsengine.cpp
    kdevelopsessionsservice.cpp
)

kde4_add_plugin(plasma_engine_kdevelopsessions ${kdevelopsessionsengine_SRCS})
target_link_libraries(plasma_engine_kdevelopsessions ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_kdevelopsessions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-kdevelopsessions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.kdevelopsessions.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

