add_definitions(-DTRANSLATION_DOMAIN=\"kdevgit\")

declare_qt_logging_category(kdevgit_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_GIT
    CATEGORY_BASENAME "git"
)
set(kdevgit_PART_SRCS
    repostatusmodel.cpp
    committoolview.cpp
    diffviewsctrl.cpp
    simplecommitform.cpp
    rebasedialog.cpp
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
ki18n_wrap_ui(kdevgit_PART_SRCS rebasedialog.ui)
kdevplatform_add_plugin(kdevgit SOURCES ${kdevgit_PART_SRCS})
target_link_libraries(kdevgit
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KF6::SonnetUi
    KF6::TextWidgets
    Qt::Core5Compat
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(icons)

install(PROGRAMS org.kde.kdevelop_git.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES kdevgit.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
