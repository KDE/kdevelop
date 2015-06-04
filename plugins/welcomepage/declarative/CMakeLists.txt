project(kdevelop-declarative)

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_library(kdevelopdashboarddeclarativeplugin SHARED kdevelopdashboarddeclarativeplugin.cpp icoreobject.cpp)

target_link_libraries(kdevelopdashboarddeclarativeplugin
    Qt5::Qml

    KDev::Vcs
)

install(TARGETS kdevelopdashboarddeclarativeplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kdevplatform)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kdevplatform)
