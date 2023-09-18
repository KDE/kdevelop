add_definitions(-DTRANSLATION_DOMAIN=\"kdevgenericprojectmanager\")
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kdevplatform_add_plugin(KDevGenericManager SOURCES ${kdevgenericmanager_PART_SRCS})

target_link_libraries(KDevGenericManager
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KDev::Project
)
