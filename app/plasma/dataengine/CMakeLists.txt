set(kdevelopsessionsengine_SRCS
    kdevelopsessionsengine.cpp
    kdevelopsessionsservice.cpp
)

add_library(plasma_engine_kdevelopsessions MODULE ${kdevelopsessionsengine_SRCS})
target_link_libraries(plasma_engine_kdevelopsessions KF5::Plasma KF5::KIOWidgets)

kcoreaddons_desktop_to_json(plasma_engine_kdevelopsessions plasma-dataengine-kdevelopsessions.desktop)

install(TARGETS plasma_engine_kdevelopsessions DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-kdevelopsessions.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install(FILES org.kde.plasma.dataengine.kdevelopsessions.operations DESTINATION ${KDE_INSTALL_DATADIR}/plasma/services)

