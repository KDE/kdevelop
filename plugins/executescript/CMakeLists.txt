add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecutescript\")

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)
ki18n_wrap_ui(kdevexecutescript_PART_SRCS
    scriptappconfig.ui
)
kdevplatform_add_plugin(kdevexecutescript JSON kdevexecutescript.json SOURCES ${kdevexecutescript_PART_SRCS})
target_link_libraries(kdevexecutescript
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)

install(FILES iexecutescriptplugin.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/executescript COMPONENT Devel)
