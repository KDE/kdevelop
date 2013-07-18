
project(ghprovider)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
find_package(KDevPlatform REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package(QJson REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
)

add_subdirectory(icons)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

set(kdevghprovider_PART_SRCS
    ghlineedit.cpp
    ghprovidermodel.cpp
    ghproviderplugin.cpp
    ghproviderwidget.cpp
    ghresource.cpp
    ghaccount.cpp
    ghdialog.cpp
)

kde4_add_plugin(kdevghprovider ${kdevghprovider_PART_SRCS})
target_link_libraries(kdevghprovider
  ${KDE4_KDEUI_LIBS} 
  ${KDEVPLATFORM_INTERFACES_LIBRARIES} 
  ${KDEVPLATFORM_VCS_LIBRARIES} 
  ${KDE4_KIO_LIBS}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
  ${KDEVPLATFORM_UTIL_LIBRARIES}
  ${QJSON_LIBRARY}
)

install(TARGETS kdevghprovider DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevghprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(cmake)
