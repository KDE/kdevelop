project(kdevelop-declarative)

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_library(kdevelopdashboarddeclarativeplugin SHARED kdevelopdashboarddeclarativeplugin.cpp icoreobject.cpp)

target_link_libraries(kdevelopdashboarddeclarativeplugin
    ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_VCS_LIBRARIES}
)

install(TARGETS kdevelopdashboarddeclarativeplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kdevplatform)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kdevplatform)
