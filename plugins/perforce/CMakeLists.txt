add_subdirectory(p4clientstub)
add_subdirectory(test)

set(kdevperforce_PART_SRCS
    perforceplugin.cpp
    perforcepluginmetadata.cpp
)

kdevplatform_add_plugin(kdevperforce JSON kdevperforce.json SOURCES ${kdevperforce_PART_SRCS})

target_link_libraries(kdevperforce
   KDev::Interfaces
   KDev::Vcs
)
