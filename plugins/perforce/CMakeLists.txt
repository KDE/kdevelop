add_subdirectory(p4clientstub)
declare_qt_logging_category(kdevperforce_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_PERFORCE
    CATEGORY_BASENAME "perforce"
)
set(kdevperforce_UIS
    ui/perforceimportmetadatawidget.ui
)
set(kdevperforce_PART_SRCS
    ui/perforceimportmetadatawidget.cpp
    perforceplugin.cpp
    perforcepluginmetadata.cpp
    ${kdevperforce_LOG_PART_SRCS}
)
ki18n_wrap_ui(kdevperforce_PART_SRCS ${kdevperforce_UIS})
kdevplatform_add_plugin(kdevperforce SOURCES ${kdevperforce_PART_SRCS})

target_link_libraries(kdevperforce
   KDev::Interfaces
   KDev::Vcs
   KF6::KIOWidgets
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
