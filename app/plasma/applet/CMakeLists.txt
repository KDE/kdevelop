plasma_install_package(package org.kde.kdevelopsessions)

set(kdevelopsessionsplugin_SRCS
    plugin/kdevelopsessionsplugin.cpp
    plugin/sessionlistmodel.cpp
)

add_library(kdevelopsessionsplugin SHARED ${kdevelopsessionsplugin_SRCS})

target_link_libraries(kdevelopsessionsplugin
    KDevelopSessionsWatch
    KF${KF_MAJOR_VERSION}::Plasma
    KF${KF_MAJOR_VERSION}::I18n
    Qt${QT_MAJOR_VERSION}::Qml
)

install(TARGETS kdevelopsessionsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kdevelopsessions)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kdevelopsessions)
