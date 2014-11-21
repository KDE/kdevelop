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
add_library(kdevgit MODULE ${kdevgit_PART_SRCS})

configure_file(kdevgit.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgit.desktop)
kcoreaddons_desktop_to_json(kdevgit ${CMAKE_CURRENT_BINARY_DIR}/kdevgit.desktop)

target_link_libraries(kdevgit
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KF5::SonnetUi
)

########### install files ###############
install(TARGETS kdevgit DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
