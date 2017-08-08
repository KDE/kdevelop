add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecute\")
########### next target ###############

set(kdevexecute_PART_SRCS

    projecttargetscombobox.cpp
    executeplugin.cpp
    nativeappconfig.cpp
    nativeappjob.cpp
)

ecm_qt_declare_logging_category(kdevexecute_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_EXECUTE
    CATEGORY_NAME "kdevplatform.plugins.execute"
)

ki18n_wrap_ui( kdevexecute_PART_SRCS nativeappconfig.ui)

kdevplatform_add_plugin(kdevexecute JSON kdevexecute.json SOURCES ${kdevexecute_PART_SRCS})

target_link_libraries(kdevexecute
    KF5::KCMUtils
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

########### install files ###############

install(FILES iexecuteplugin.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/execute COMPONENT Devel)
