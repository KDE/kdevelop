add_subdirectory(p4clientstub)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(kdevperforce_PART_SRCS
    perforceplugin.cpp
    perforcepluginmetadata.cpp
)

kdevplatform_add_plugin(kdevperforce JSON kdevperforce.json SOURCES ${kdevperforce_PART_SRCS})

target_link_libraries(kdevperforce
   KDev::Interfaces
   KDev::Vcs
)
