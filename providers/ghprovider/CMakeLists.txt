project(ghprovider)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(icons)

set(kdevghprovider_PART_SRCS
    ghlineedit.cpp
    ghprovidermodel.cpp
    ghproviderplugin.cpp
    ghproviderwidget.cpp
    ghresource.cpp
    ghaccount.cpp
    ghdialog.cpp
)

add_library(kdevghprovider MODULE ${kdevghprovider_PART_SRCS})
target_link_libraries(kdevghprovider
  KF5::KIOWidgets
  ${KDEVPLATFORM_INTERFACES_LIBRARIES}
  ${KDEVPLATFORM_VCS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
  ${KDEVPLATFORM_UTIL_LIBRARIES}
)

install(TARGETS kdevghprovider DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevghprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevghprovider.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevghprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})
