add_definitions(-DTRANSLATION_DOMAIN=\"kdevgit\")

declare_qt_logging_category(kdevgit_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_GIT
    CATEGORY_BASENAME "git"
)
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
    ${kdevgit_LOG_PART_SRCS}
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

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(icons)

install(PROGRAMS org.kde.kdevelop_git.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES kdevgit.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
