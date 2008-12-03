#@todo: this file needs to be worked over

########### next target ###############

set(diff2_PART_SRCS
    kompareprocess.cpp
    komparemodellist.cpp
    diffmodellist.cpp
    diffmodel.cpp
    difference.cpp
    diffhunk.cpp
    levenshteintable.cpp
    parser.cpp
    parserbase.cpp
    cvsdiffparser.cpp
    diffparser.cpp
    perforceparser.cpp
    diffsettings.cpp
    settingsbase.cpp )


# This library only exists as a convenience library for the teamwork plugin, thus it is not SOVERSION'ed
kde4_add_library(kdevteamwork_diff2 SHARED ${diff2_PART_SRCS})

target_link_libraries(kdevteamwork_diff2  ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kdevteamwork_diff2 ${INSTALL_TARGETS_DEFAULT_ARGS})
