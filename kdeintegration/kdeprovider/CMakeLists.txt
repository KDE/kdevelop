add_definitions(-DTRANSLATION_DOMAIN=\"kdevkdeprovider\")
add_subdirectory(tests)

set(kdevkdeprovider_PART_SRCS kdeprojectsmodel.cpp
    kdeproviderwidget.cpp
    kdeproviderplugin.cpp
    kdeprojectsmodel.cpp
    kdeprojectsreader.cpp
)
kconfig_add_kcfg_files(kdevkdeprovider_PART_SRCS kdeconfig.kcfgc)
ki18n_wrap_ui(kdevkdeprovider_PART_SRCS kdeconfig.ui)

kdevplatform_add_plugin(kdevkdeprovider JSON kdevkdeprovider.json SOURCES ${kdevkdeprovider_PART_SRCS})
target_link_libraries(kdevkdeprovider KF5::ItemViews KDev::Interfaces KDev::Vcs)
