add_definitions(-DTRANSLATION_DOMAIN=\"kdevgit\")

add_subdirectory(tests)
add_subdirectory(icons)

set(kdevgit_PART_SRCS
    stashmanagerdialog.cpp
    stashpatchsource.cpp
    gitmessagehighlighter.cpp
    gitclonejob.cpp
    gitplugin.cpp
    gitpluginmetadata.cpp
    gitjob.cpp
    gitplugincheckinrepositoryjob.cpp
    gitnameemaildialog.cpp
)
ki18n_wrap_ui(kdevgit_PART_SRCS stashmanagerdialog.ui)
ki18n_wrap_ui(kdevgit_PART_SRCS gitnameemaildialog.ui)
kdevplatform_add_plugin(kdevgit JSON kdevgit.json SOURCES ${kdevgit_PART_SRCS})
target_link_libraries(kdevgit
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KF5::SonnetUi
)
