project(ghprovider)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevghprovider\")
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

kdevplatform_add_plugin(kdevghprovider JSON kdevghprovider.json	SOURCES ${kdevghprovider_PART_SRCS})
target_link_libraries(kdevghprovider
  KF5::KIOWidgets
  KF5::KDELibs4Support
  KDev::Interfaces
  KDev::Vcs
  KDev::Language
  KDev::OutputView
  KDev::Util
)
