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
ecm_qt_declare_logging_category(kdevghprovider_PART_SRCS
    HEADER debug.h
    IDENTIFIER GHPROVIDER
    CATEGORY_NAME "kdevelop.providers.ghprovider"
)

kdevplatform_add_plugin(kdevghprovider JSON kdevghprovider.json	SOURCES ${kdevghprovider_PART_SRCS})
target_link_libraries(kdevghprovider
  KF5::KIOWidgets
  KDev::Interfaces
  KDev::Vcs
  KDev::OutputView
  KDev::Util
)
