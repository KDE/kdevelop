project(kdevelop-declarative)

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_library(kdevelopdashboarddeclarativeplugin SHARED kdevelopdashboarddeclarativeplugin.cpp icoreobject.cpp)

target_link_libraries(kdevelopdashboarddeclarativeplugin
    Qt5::Core Qt5::Gui Qt5::Declarative
    
    ${KDEVPLATFORM_VCS_LIBRARIES}
)

install(TARGETS kdevelopdashboarddeclarativeplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kdevplatform)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kdevplatform)
