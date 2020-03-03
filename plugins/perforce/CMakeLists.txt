# temporary while patch series in work 08/2018
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

add_subdirectory(p4clientstub)
declare_qt_logging_category(kdevperforce_LOG_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_PERFORCE
    CATEGORY_BASENAME "perforce"
)
set(kdevperforce_UIS
    ui/perforceimportmetadatawidget.ui
)
ki18n_wrap_ui(kdevperforce_UI_PART_SRCS ${kdevperforce_UIS})
set(kdevperforce_PART_SRCS
    ui/perforceimportmetadatawidget.cpp
    perforceplugin.cpp
    perforcepluginmetadata.cpp
    ${kdevperforce_UI_PART_SRCS}
    ${kdevperforce_LOG_PART_SRCS}
)
kdevplatform_add_plugin(kdevperforce JSON kdevperforce.json SOURCES ${kdevperforce_PART_SRCS})

target_link_libraries(kdevperforce
   KDev::Interfaces
   KDev::Vcs
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
