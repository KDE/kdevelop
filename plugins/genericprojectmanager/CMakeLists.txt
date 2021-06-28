add_definitions(-DTRANSLATION_DOMAIN=\"kdevgenericprojectmanager\")
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kdevplatform_add_plugin(kdevgenericmanager SOURCES ${kdevgenericmanager_PART_SRCS})

target_link_libraries(kdevgenericmanager
    KF5::KIOWidgets
    KDev::Project
)
