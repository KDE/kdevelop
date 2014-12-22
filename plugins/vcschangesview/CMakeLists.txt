add_definitions(-DTRANSLATION_DOMAIN=\"kdevvcsprojectintegration\")
project(vcschangesview)

set(vcschangesview_SRCS
    vcschangesview.cpp

    vcschangesviewplugin.cpp
)

kdevplatform_add_plugin(kdevvcschangesviewplugin JSON kdevvcschangesview.json SOURCES ${welcomepage_SRCS})

target_link_libraries(kdevvcschangesviewplugin
    KDev::Interfaces
    KDev::Project
    KDev::Vcs
)
