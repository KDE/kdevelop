add_definitions(-DTRANSLATION_DOMAIN=\"kdevghprovider\")

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
declare_qt_logging_category(kdevghprovider_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER GHPROVIDER
    CATEGORY_BASENAME "ghprovider"
    DESCRIPTION "githubprovider"
)

kdevplatform_add_plugin(kdevghprovider SOURCES ${kdevghprovider_PART_SRCS})
target_link_libraries(kdevghprovider
  KF6::KIOWidgets
  KDev::Interfaces
  KDev::Vcs
  KDev::OutputView
  KDev::Util
)
