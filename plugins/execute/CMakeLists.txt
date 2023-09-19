add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecute\")
########### next target ###############

set(kdevexecute_PART_SRCS

    projecttargetscombobox.cpp
    executeplugin.cpp
    nativeappconfig.cpp
    nativeappjob.cpp
)

declare_qt_logging_category(kdevexecute_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_EXECUTE
    CATEGORY_BASENAME "execute"
)

ki18n_wrap_ui( kdevexecute_PART_SRCS nativeappconfig.ui)

kdevplatform_add_plugin(kdevexecute SOURCES ${kdevexecute_PART_SRCS})

target_link_libraries(kdevexecute
    KF6::KCMUtils
    KF6::KIOWidgets
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

add_library(KDevPlatformIExecute INTERFACE)
add_library(KDev::IExecute ALIAS KDevPlatformIExecute)
target_include_directories(KDevPlatformIExecute INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevplatform>"
)
set_target_properties(KDevPlatformIExecute PROPERTIES
    EXPORT_NAME IExecute
)
install(TARGETS KDevPlatformIExecute EXPORT KDevPlatformTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES iexecuteplugin.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/execute COMPONENT Devel)
