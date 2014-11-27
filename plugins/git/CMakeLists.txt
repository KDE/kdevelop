add_definitions(-DTRANSLATION_DOMAIN=\"kdevgit\")
project(git)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${git_SOURCE_DIR}/cmake)

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
)

ki18n_wrap_ui(kdevgit_PART_SRCS stashmanagerdialog.ui)
kdevplatform_add_plugin(kdevgit JSON kdevgit.json SOURCES ${kdevgit_PART_SRCS})

target_link_libraries(kdevgit
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KF5::SonnetUi
)
