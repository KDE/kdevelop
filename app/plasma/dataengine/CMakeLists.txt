set(kdevelopsessionsengine_SRCS
    kdevelopsessionsengine.cpp
    kdevelopsessionsservice.cpp
)

add_library(plasma_engine_kdevelopsessions MODULE ${kdevelopsessionsengine_SRCS})
target_link_libraries(plasma_engine_kdevelopsessions KF5::Plasma KF5::KIOWidgets KF5::KDE4Support)

install(TARGETS plasma_engine_kdevelopsessions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-kdevelopsessions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.kdevelopsessions.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

