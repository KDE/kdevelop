add_definitions(-DTRANSLATION_DOMAIN=\"kdevkonsole\")
########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleviewplugin.cpp
    kdevkonsoleview.cpp
)

kdevplatform_add_plugin(kdevkonsoleview JSON kdevkonsoleview.json SOURCES ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview KF5::Parts KDev::Interfaces)
