install(DIRECTORY package/
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/kdevelopsessions)

install(FILES package/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-applet-kdevelopsessions.desktop)

set(kdevelopsessionsplugin_SRCS
    plugin/kdevelopsessionsplugin.cpp
    plugin/sessionlistmodel.cpp
)

add_library(kdevelopsessionsplugin SHARED ${kdevelopsessionsplugin_SRCS})

target_link_libraries(kdevelopsessionsplugin
    KDevelopSessionsWatch
    KF5::Plasma
    KF5::I18n
    Qt5::Qml
)

install(TARGETS kdevelopsessionsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kdevelopsessions)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kdevelopsessions)
