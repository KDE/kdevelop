add_definitions(-DTRANSLATION_DOMAIN=\"kdevpastebin\")
set(kdevpastebin_PART_SRCS
    pastebinplugin.cpp
)

kdevplatform_add_plugin(kdevpastebin JSON kdevpastebin.json SOURCES ${kdevpastebin_PART_SRCS})
target_link_libraries(kdevpastebin KDev::Vcs)
