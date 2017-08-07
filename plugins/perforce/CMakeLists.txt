add_subdirectory(p4clientstub)
ecm_qt_declare_logging_category(kdevperforce_LOG_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_PERFORCE
    CATEGORY_NAME "kdevplatform.plugins.perforce"
)
set(kdevperforce_PART_SRCS
    perforceplugin.cpp
    perforcepluginmetadata.cpp
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
