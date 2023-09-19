add_library(kdevelopdashboarddeclarativeplugin SHARED kdevelopdashboarddeclarativeplugin.cpp icoreobject.cpp)
target_link_libraries(kdevelopdashboarddeclarativeplugin
    Qt${QT_MAJOR_VERSION}::Qml
    KDev::Vcs
)

install(TARGETS kdevelopdashboarddeclarativeplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kdevplatform)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kdevplatform)
