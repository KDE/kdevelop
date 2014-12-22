add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecute\")
########### next target ###############

set(kdevexecute_PART_UIS
    nativeappconfig.ui
)

set(kdevexecute_PART_SRCS

    projecttargetscombobox.cpp
    executeplugin.cpp
    nativeappconfig.cpp
    nativeappjob.cpp
)

ki18n_wrap_ui( kdevexecute_PART_SRCS ${kdevexecute_PART_UIS} )

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

install(FILES iexecuteplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/execute COMPONENT Devel)
